# API 一致性反模式 (API Consistency Anti-Patterns)

> api-scanner 专用的 API 一致性缺陷模式词典。

---

## API-001 资料 × 接口定义 × 实现 签名不一致

**信号特征**
- 方法名、参数名、参数类型、参数顺序、返回值类型：docs × d.ts/.h × 框架实现 三者不一致。
- 参数描述（含义、必填）不一致：docs 参数表 × d.ts 可选参数 `?`。
- const 修饰、枚举/typedef 定义不一致。

**典型后果**：开发者照文档误用 API，P1。

---

## API-002 资料 × 接口定义 × 实现 起始版本不一致（最高频出错点）

**信号特征**
- docs 章节版本号 × d.ts `@since X dynamic` × `lib*.ndk.json` `first_introduced` 三者数值不一致。
- 抄版本时凭记忆（曾把 `setColorMode` 写成 13，实际 d.ts 与 docs 均为 18）。

**典型后果**：版本兼容性误判，P1。

---

## API-003 资料 × 接口定义 × 实现 错误码不一致

**信号特征**
- docs 错误码表 × d.ts `@throws` × 实现 三者不一致。
- docs 错误码表未覆盖 d.ts `@throws` 所有错误码。
- 语义反转（文档说返回 X 实现返回 Y，如应 401 实际 16000001）。
- 设备行为差异错误码未在错误码表中列出。

**典型后果**：错误处理误判，P1。

---

## API-004 框架实现参数校验与接口定义不一致

**信号特征**
- 参数校验未覆盖空指针/非法值/类型不符。
- 错误码返回与 d.ts `@throws`/`.h` 声明不一致（非法参数应 401 而非业务错误码）。
- C API 销毁函数未将指针置 null（双指针 vs 单指针）。
- 字符串/缓冲区长度校验不完整。
- system api 标注不一致。

**典型后果**：调用方误判、安全绕过，P0/P1。

---

## API-005 资料 E 项：描述与实现行为不一致（最高价值）

**E1 描述缺失/需补充说明**：docs 描述过于简略，关键行为未说明，开发者照文档用会踩坑。
- 例：`openLink` docs 只说"打开链接"，实际触发原子服务免安装下载流程。

**E2 隐含限制未声明**：代码有 `CHECK_CALLER_IS_SYSTEM_APP`/`IsForegroundCheck`/特定 bundleName 白名单，docs 完全未提及。
- 例：方法实际仅系统应用可调，但 docs 无 `@systemapi` 标注。

**E3 副作用未声明**：调用产生对外可见状态变化或回调，docs 只描述主动行为。
- 例：`IsEmbeddedOpenAllowed`（查询语义）内部调用 `StartFreeInstall` 拉起免安装。

**E4 描述与实现不符**：docs 描述 A，代码实现 B。**一律 ❌ 最高优先级**。
- 例：docs 说"返回结果通过 Promise resolve"，实际 reject。
- 例：docs 说参数 `options.timeout` 生效，实际代码完全忽略。

**E5 描述冗余/过时**：docs 描述了已废弃字段、已删除参数、不再生效逻辑。

**E6 描述模糊导致误用**：关键概念未定义清楚。

**E7 行为时序未说明**：异步 API 回调时序、并发调用串行化保证未体现。

**判定与定级**
- E1/E2/E3：根据影响定级——导致调用失败或预期外行为 = ❌；仅影响理解 = ⚠️。
- E4：一律 ❌（最高优先级，开发者照文档必然踩坑）。
- E5/E6/E7：默认 ⚠️；若直接导致错误使用则 ❌。

**每条发现必须给出**：docs md file:line（描述所在）+ 代码 file:line（实际行为所在）+ 一句话差异说明。

---

## API-006 资料存在性与内容完整性

**信号特征**
- 存在性（首要）：d.ts 声明但 docs 缺章节（`### <方法名>`）→ ❌ 资料缺失 P0。
- 完整性：参数表四列（名/类型/必填/描述）不齐全、返回值表缺失、示例代码缺失。

> docs md 的 markdown 格式问题（`<sup>` 闭合、`<br/>` 等）不在审计范围。

---

## API-007 框架实现内部 bug（基础）

**信号特征**
- 内存分配/释放配对错误。
- 整数溢出（`strlen` 赋值给 `int32_t`）。
- 浅拷贝 vs 深拷贝（特别是 `char*`）。
- 拼写错误（函数名、常量名、注释）。

---

## API-008 服务侧生命周期/线程安全

**信号特征**
- lambda 捕获栈变量引用/裸指针，异步回调访问已析构对象。
- RAII guard 析构无条件执行副作用（如发出回调）。
- 回调 ID/handle 生成可能碰撞。
- 静态 bool 标志位无锁检查。
- 回调 map `operator[]` 覆盖 vs `insert_or_assign`。
- 锁顺序全局不一致。
- `signal()` vs `sigaction`，`waitpid(-1)` 收割任意子进程。

---

## API-009 服务侧资源泄漏

**信号特征**
- 异常路径 fd 未关闭（open 成功后续失败）。
- 异常路径内存未释放（new 后 return 错误）。
- 异常路径锁未释放。
- 回调对象在超时/失败路径未清理。
- 嵌套容器反序列化中途失败已分配子容器未释放。
- `Variant_Clear` 未覆盖所有类型分支。

---

## API-010 服务侧 IPC 序列化

**信号特征**
- `ReadFromParcel`/`WriteToParcel` 字段顺序不匹配。
- 枚举值直接 `static_cast` 不做范围校验。
- size 字段未校验负数。
- fd 通过 Parcel 传递时所有权问题。
- TOCTOU。

---

## API-011 服务侧 API 行为/副作用

**信号特征**
- 文档未声明的隐含限制（如"仅支持主进程调用"）。
- 未初始化时静默成功 vs 文档声明返回 INTERNAL。
- const 引用参数被修改（数据竞争）。
- 权限校验在特例化路径被绕过。

---

## API-012 框架实现文件静态/动态混淆（最高优先级陷阱）

**信号特征**
- 把 `ets_*.cpp`（ArkTS/ANI）、`cj_*.cpp`（Cangjie/FFI）当成 dynamic JS 实现来扫。
- 同一接口类在 `frameworks/native/.../ability_runtime/` 下常并存 `js_`/`ets_`/`cj_` 三份实现文件，只扫 `js_` 那份。

**判定口诀**：`js_` 前缀 + NAPI 签名（`napi_env`/`napi_callback_info`）= 动态（扫）；`ets_`/`cj_` 前缀或位于 `ets/`、`cj/` 目录 = 静态（跳过）。

---

## API-013 JS API 框架实现路径用 BUILD.gn relative_install_dir 区分新旧同名模块

**信号特征**
- 同一 Kit 常有新旧两套 NAPI 模块并存，目录名仅差一个层级，极易找错。
- 典型：`@ohos.app.ability.appManager.d.ts`（新 Stage）vs `@ohos.application.appManager.d.ts`（旧 FA）。
  - 新接口：`frameworks/js/napi/app/js_app_manager/` → `relative_install_dir = "module/app/ability"`
  - 旧接口：`frameworks/js/napi/app/app_manager/` → `relative_install_dir = "module/application"`

**判定流程**
1. 先看 `.d.ts` namespace：`@ohos.app.ability.X` 是新接口，`@ohos.application.X` 是旧接口。
2. 在候选目录的 `BUILD.gn` 中找 `relative_install_dir`。
3. `"module/app/ability"` 对应新接口；`"module/application"` 对应旧接口。
4. 仍不确定看 `nm_modname` 或绑定 `.d.ts` namespace 字符串。

**强制要求**：写报告前必须 Grep/Read 核对 `relative_install_dir`，证明所选框架目录与 `.d.ts` 一一对应，否则整个第二轮扫描结论会作废。
