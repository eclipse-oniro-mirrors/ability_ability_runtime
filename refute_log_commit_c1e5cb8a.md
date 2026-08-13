# Refute 审查记录

> 对 commit `c1e5cb8a7be7ec2ae8a958794bdb66207c74abb8`（move process principles）的 scanner 原始发现做对抗性验证。

## 审查概要

| 指标 | 数值 |
|------|------|
| 审查范围 | logic: 3 条, security: 3 条, input: 1 条 |
| P0/P1 全审 | 1 条（LOG-01 P1）|
| P2 抽样审 | 2 条 |
| P3 不进入 refute | 3 条 |
| 维持 | 5 条 |
| 降级 | 1 条（SEC-01: P2 → P3）|
| 推翻 | 0 条 |
| 合并 | 1 组（LOG-02 吸收 requestId=0 根因）|
| **进入最终报告** | **6 条** (P0: 0, P1: 1, P2: 2, P3: 3) |

---

## 逐条审查

### ✅ 维持 — LOG-01 (P1)：4 个测试/模糊测试目标编译失败

[发现摘要] `StartSpecifiedAbility` 签名由 5 参数（含默认值）改为 3 参数结构体 `StartSpecifiedParam`（无默认值），但 4 个测试/模糊测试文件未同步更新，仍以 2 参数调用 → 编译失败。

审查结论：维持 P1。
- 触发路径证伪：确认无 2 参数重载残留。`ams_mgr_scheduler.h:307-309`、`app_mgr_client.h:568`、`app_scheduler.h:480`、`ams_mgr_interface.h:289` 均为 3 参数且无默认值；grep 全库无 2 参数声明。
- 影响证伪：非运行时崩溃，但阻塞 `run -t UT -tp ability_runtime`（AGENTS.md Minimum checks）的编译，违反 Done definition。4 个目标均为 `ohos_unittest`/`ohos_fuzztest`（BUILD.gn 已确认）。
- 证据：`ams_mgr_scheduler_test.cpp:764,767`、`app_mgr_client_test.cpp:574`、`startspecifiedability_fuzzer.cpp:54`、`amsmgrscheduler_fuzzer.cpp:123`。
无变化。

### ✅ 维持 — LOG-02 (P2)：mission_list_manager 路径 requestId=0 导致 NEW_PROCESS 进程名碰撞

[发现摘要] `mission_list_manager.cpp:224-231` 构造 `StartSpecifiedParam` 时未设置 `requestId`（默认 0）。若 `processOptions->processMode` 为 NEW_PROCESS 模式，`ResolveProcessName`→`GenerateNewProcessName` 生成 `bundle:module:name:0`，所有同类调用共用 `...:0` 进程名 → 多个 NEW_PROCESS 实例并入同一进程，破坏进程隔离。

审查结论：维持 P2。
- 触发路径证伪：路径可达性 — `isSpecified`（SPECIFIED launchMode）与 NEW_PROCESS processMode 正交，可共存。但需要 SPECIFIED + NEW_PROCESS 组合，触发条件较窄。其他路径（ui_ability_lifecycle_manager）均设 `requestId = RequestIdUtil::GetRequestId()`（唯一计数器，request_id_util.cpp:22-29，线程安全），唯独本路径漏设。维持。
- 影响证伪：后果为进程共享（正确性），非崩溃/权限绕过。NEW_PROCESS 主要是应用架构特性而非安全沙箱。维持 P2（非 P1）。
- 根因合并：requestId=0 是本问题的根因，已吸收进 LOG-02，不再单列 GenerateNewProcessName 条目。

### ✅ 维持 — SEC-02 (P2)：StartSpecifiedAbility IPC 序列化格式硬改，跨版本静默误解析

[发现摘要] `ams_mgr_proxy.cpp`/`ams_mgr_stub.cpp` 将 IPC 载荷由 `WriteInt32(requestId)+WriteString(customProcess)+WriteBool(isPreloadStart)` 改为 `WriteParcelable(&StartSpecifiedParam)`，无版本协商。

审查结论：维持 P2。
- 触发路径证伪：IAmsMgr 为 inner_api，调用方为系统进程；OpenHarmony 单仓构建下 proxy/stub 同步更新，但 OTA 分阶段升级时存在版本偏差窗口。维持。
- 影响证伪：新 stub 收旧 client 调用 — `ReadParcelable<StartSpecifiedParam>` 读到错位字段，但 `ReadFromParcel` 恒返回 true → 非空（垃圾）param，静默继续（不报错）；旧 stub 收新 client — `ReadBool` 读 processMode(int32) → bool 误读。无崩溃但行为偏差。维持 P2。

### ⬇️ 降级 — SEC-01: P2 → P3（LoadParam 序列化追加字段）

[发现摘要] `param.cpp` `LoadParam::MarshallingTwo`/`ReadFromParcel` 末尾追加 `processMode`、`requestId` 两个 Int32，无版本标志，违反 AGENTS.md "不要修改 IPC Stub/Proxy 的序列化格式"。

审查结论：降级为 P3。
- 影响夸大证伪：新字段位于 `MarshallingTwo` 末尾（`Marshalling` 末尾调用 `MarshallingTwo`）。
  - 新 writer → 旧 reader：旧 reader 读到 reusePid 后停止，尾部 2 个 Int32 为未读剩余字节；LoadParam 通常是 parcel 中最后对象，剩余字节无害。
  - 旧 writer → 新 reader：`ReadInt32` 在耗尽的 parcel 上返回 0 → processMode=0(UNSPECIFIED)、requestId=0，均为安全默认值（下游 `ConvertInt32ToProcessMode` 再校验）。
- 无崩溃、无安全破绽、默认值安全。仍是 IPC 格式变更（违反不变量），但实际影响极低 → 降级 P3。

### ✅ 维持 — INP-01 (P3)：StartSpecifiedParam::ReadFromParcel 恒返回 true

[发现摘要] `param.cpp:173-180` `ReadFromParcel` 无条件 `return true`；耗尽/截断 parcel 的 `ReadInt32/ReadString/ReadBool` 返回默认值而非报错。

审查结论：维持 P3。
- 影响证伪：默认值（requestId=0, customProcess="", processMode=0, isPreloadStart=false）经下游 `ConvertInt32ToProcessMode`（process_options.cpp:87-94，范围校验→UNSPECIFIED）后安全；无 NEW_PROCESS 触发、无 preload。且与 `LoadParam::ReadFromParcel`（同样恒 true）一致，属本仓惯例。
- 维持 P3（健壮性观察，非安全破绽）。

### ✅ 维持 — TEST-01 (P3)：ams_mgr_stub_test 0200/0300 用 WriteBool(false) 模拟 null 指针

[发现摘要] `ams_mgr_stub_test.cpp` 0200/0300 用 `data.WriteBool(false)` 作为 "null want/abilityInfo 指示"，但 stub 实际用 `ReadParcelable<Want>` 读取，bool 字节会被误读为 Want 字段，未必能可靠触发 null 返回路径。

审查结论：维持 P3（测试健壮性，仅测试代码）。

---

## refute 统计
- 审查耗时：约 4 分钟
- 调用链追踪：6 次（签名 grep、调用点 grep、mock 确认、ConvertInt32ToProcessMode、RequestIdUtil、MakeProcessName 语义）
- 代码片段二次验证：8 处
