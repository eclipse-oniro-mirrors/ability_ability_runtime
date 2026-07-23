# 已知缺陷模式库（历史 257+ 条已闭环问题单提炼）

> 用途：安全审计时对历史高发模式做"同类排查"。每个模式给出特征信号（含 grep 线索）、真实历史案例、检查点。
> 原则：**发现一处，全库横扫**——这些模式在历史上都是批量出现的。

## 目录
- G1 身份信任错误
- G2 鉴权遗漏分支
- G3 反序列化缺"四件套"
- G4 同族 API 模式化误用
- G5 user 版本隐藏面
- G6 路径穿越与 Zip Slip（含修复被绕过）
- G7 日志红线
- G8 配置级提权
- G9 沙箱隔离失效
- G10 签名/安装管控绕过
- 附：历史复发热点模块清单

---

## G1 身份信任错误

**特征信号 / grep 线索**
- `getpid` / `GetCallingPid` / `pid` 作为身份或权限判断依据；未校验 pid 所属 uid/token 是否变化。
- 从客户端传入的 `tokenId` / `fullTokenId` / `specifiedTokenId` 直接采信，未经服务端映射校验。
- 依赖 `binder death recipient` / fd 状态推断对端存活或身份。
- 序列化端与反序列化端对同一参数的处理逻辑不一致（走私攻击面）。

**历史案例**
- `UninstallAppInner` 用 pid 校验，pid 回绕 → 卸载任意应用。
- `ExecuteIntentForDistributed` 的 `specifiedFullTokenId` 外部可控 → 任意 token 身份伪造。
- AMS tokenId 注入 → Launch-Any-Page；任意进程 tokenId 可提取 → 仿冒系统应用下发指令。
- appmgr 完全信任 binder death notification → close fd 实现进程保活。
- CLI-SA 与沙箱间序列化/反序列化参数处理不一致 → 命令行参数走私。

**检查点**
- 所有身份判断是否最终落到 uid/token（服务端可验证），而非 pid/自报字段？
- 跨进程传递的 tokenId 是否经 `AccessTokenKit` 类机制验真？
- 进程存活/会话有效性是否依赖可被对端操纵的信号？

## G2 鉴权遗漏分支

**特征信号 / grep 线索**
- Stub 的 `OnRemoteRequest`/`OnRemoteRequestEx` 中部分 code 分支无权限校验直接进业务。
- 白名单判断存在可绕过的类型/前缀分支（如以 AGENT 类型扩展绕过 SA 白名单）。
- 跨用户（multi-user / 跨空间）场景只校验调用者身份，未校验目标资源所属 userId。
- 组件 `exported=false`、调试开关（开发者模式/USB 调试/框架调试开关）关闭时仍可被调用。

**历史案例**
- `GetApplicationInfo` 等查询接口缺跨用户校验 → 跨用户应用数据泄露。
- `CliToolManagerService::ExecTool` 存在未鉴权路径 → 任意 Hap 以 CLI-SA 身份执行任意 Skill。
- `StartUIExtensionAbility`/`ConnectUIExtensionAbility` 未校验调用者 → 跨用户连接任意 App 组件。
- 拉起 invisible 组件时对 SA 放通 → 任意 SA 拉起非导出 Ability。
- 意图调试 hap 不校验开发者模式/USB 调试/调试开关 → 三方应用直接调用。

**检查点**
- 逐 code/逐接口核对：每个对外 IPC 入口是否都有权限声明？默认是否拒绝？
- 白名单/豁免分支能否被构造输入绕过？
- userId 是否始终取自服务端上下文而非客户端参数？

## G3 反序列化缺"四件套"

**特征信号 / grep 线索**
- `ReadFromParcel` / `Unmarshalling` / JSON 解析 / 自定义二进制解析中：
  - 从 parcel 读出的 int32/size/count 直接用于 `resize`/`new`/循环次数（无上限校验）。
  - 递归解析嵌套结构无深度计数。
  - 按外部 length 循环读取无总次数/超时限制。
  - `std::stoi`/`stol`、类型转换无 try-catch。
- 解析逻辑位于权限校验代码**之前**（未授权即可触发）。

**历史案例**
- `SkillExecuteResult::ReadFromParcel` 未校验 uriCount → IPC 致 foundation CPU/内存耗尽（鉴权前触发，任意三方 App 可打）。
- `InsightIntentExecuteResult::ReadFromParcel` int32 直传 `resize` → OOM。
- `ParseWantParams` 递归无深度限制 → 开机加载通知 want 时栈耗尽 crash（不开机风险）。
- 44/79 字节 JSON → `ParseWantParamsWithBrackets` 死循环 600s+。
- `ReadFromParcel` 缺容器大小上限 → 内存放大；`EncodeBase64(srcLen=0)` 堆越界。

**检查点**
- 每个解析入口逐一核对四件套：长度/数量上限、递归深度、循环上限、异常兜底。
- 解析调用点是否可能先于鉴权执行？

## G4 同族 API 模式化误用

**特征信号 / grep 线索**
- `napi_open_handle_scope`、`napi_create_reference`、`napi_coerce_to_native_binding_object`、`napi_queue_async_work_with_qos`、`napi_wrap` 调用后不判断 scope 值与返回值。
- `iface_cast` 使用点（历史上单模块 500+ 处误用）。
- 智能指针手动 `get()` 后 delete / 托管内存被二次释放（double free）。

**历史案例**
- 同一 NAPI scope 误用在 js_ui_appearance / napi_context / napi_async_work_callback 等多文件重复出现 → 内存泄漏。
- `dbmsi` 两文件智能指针使用错误 → double free。
- `environment_callback.cpp` NAPI 引用泄漏 → Use-After-Free。

**检查点**
- 命中一处即 grep 全库同族 API，按文件分组列出。
- 该 API 是否有封装缺失（应加 wrapper 强制返回值检查）？

## G5 user 版本隐藏面

**特征信号 / grep 线索**
- CLI 工具的命令/参数解析表中存在文档未登记的命令或 flag；`--help` 输出与代码支持集不一致。
- user/RELEASE 版本保留调试入口、测试 hook、`-D` 类调试参数。
- "扫描仪驱动/debug 签名"类限制判断可被构造条件绕过。

**历史案例**
- `aa` 隐藏参数 `-s`（crash）、`-C`（冷启动）、`-c`（持续启动）、`stop-service` 隐藏参数、`force-stop` 杀非 debug 签名进程、`pre-start` 未文档化。
- `ability_tool -D` 对 release 签名应用进入调试模式；`bm` 存在未公开子命令。
- 双空间禁装 debug 签名驱动的判断被绕过。

**检查点**
- 导出代码支持的命令/参数全集，与对外文档 diff，未登记项全部上报。
- 调试/测试入口在 user 版本是否有编译期或运行期硬隔离？

## G6 路径穿越与 Zip Slip（含修复被绕过）

**特征信号 / grep 线索**
- 外部输入拼入文件路径前无规范化（`realpath`/`canonicalize`）与白名单前缀校验。
- ZIP 解压条目名未校验 `..`（Zip Slip）。
- 临时目录使用可预测路径 + 非独占创建（TOCTOU 竞争）。
- 安装/卸载流程残留中间目录（HNP 类）可被注入。
- 同一函数存在"针对某个 POC 的特判修复"痕迹。

**历史案例**
- `FileUtils.unzipFile` Zip Slip → 覆盖目标目录外任意文件。
- hnp 解压路径穿越，**原修复被绕过**（两轮复发）；BMS 文件名/路径外部可控 → 复制指定路径文件、致其他应用不可用。
- 临时目录竞争 → `/data/system/` 内容被 skill 文件覆盖。
- installs 向 foundation 暴露任意路径文件写/复制原语 → 进程拆分纵深防御失效（架构级）。

**检查点**
- 路径处理是否收敛到统一规范化+白名单函数？散落的点位拼接全部上报。
- 修复方式审查：特判封堵按"可被绕过"上报。

## G7 日志红线

**特征信号 / grep 线索**
- 日志打印：`udid`、`networkid`、`SN`、`challenge`、`token`、`password`、完整 `cmdLine`、文件路径/名称、已安装应用包名。

**历史案例**
- `distributed_data_storage.cpp` 打印 udid；`ability_record.cpp`/`ui_ability_lifecycle_manager.cpp` 打印 networkid；aa/AMS 打印 SN（多版本复发）；`process_manager.cpp` 打印完整 cmdLine（含会话 challenge）；climgr 多处敏感日志；Context 打印文件名。

**检查点**
- 对红线关键字做全库 grep，逐处确认是否脱敏。
- 凭据类内容出现在日志中：除报代码问题外，提示"凭据视同已泄露，需评估轮换"。

## G8 配置级提权

**特征信号 / grep 线索**
- SELinux 策略中服务数据库/目录标签过宽；CLI/bin 无独立标签（shell 可直接执行）。
- 权限映射表（如 cli_permission_map.json）中单个权限映射多个系统权限。
- PermissionDefinitions.json 与对外文档的 `availableType`/ACL 使能方式不一致。
- 系统公共事件已定义但未加入系统公共事件列表（历史多次重犯）。
- 系统应用声明与功能无关的权限（违反最小化，如弹窗应用申请联网）。

**历史案例**
- `auto_startup_service.db` / `ability_manager_service.db` / `bmsdb.db` 标签过大 → 攻破低权限服务即可篡改开机自启、MDM 保活。
- `ohos.permission.cli.START_ABILITY` 含 `START_INVISIBLE_ABILITY` → 调起任意应用非暴露组件。
- 小艺 Claw CLI 无独立标签 → shell 无权限执行任意 claw 指令。

**检查点**
- 审查不只看代码：SELinux 策略、权限定义、映射表、事件列表均纳入范围。
- 发现"权限/标签比功能需要的宽"即上报。

## G9 沙箱隔离失效

**特征信号 / grep 线索**
- 创建子进程/沙箱未配置 pid 等 namespace 隔离。
- 终止沙箱进程仅发 `SIGTERM`（可被忽略/捕获），无 `SIGKILL` 兜底。
- fork/exec 前未关闭继承的 fd（pipefd 遗留）。
- 会话（session）标识可被猜测或未绑定创建者身份。

**历史案例**
- CLI-SA 拉起 claw_sandbox 未隔离 pid namespace → killpg 超时机制失效，沙箱进程长期驻留。
- killpg 用 SIGTERM → 恶意进程忽略信号不被杀。
- 父进程遗留 pipefd → 沙箱内进程接管其他 Session 输入输出。
- sessionId 泄露 → 跨应用 CLI 会话劫持；全局变量竞争 → 跨应用泄露 Skill 执行结果。
- nativespawn 孵化进程沙箱问题。

**检查点**
- 沙箱创建五件套：namespace 隔离、fd 关闭清单、SIGKILL 兜底、会话所有权绑定 uid、最小权限。
- 会话 id 是否不可猜测、是否校验所有权？

## G10 签名/安装管控绕过

**特征信号 / grep 线索**
- 代码签名/验签流程存在"豁免"配置（ownerid 豁免等）。
- 解析可执行文件（abc/zip）时未按签名保护的信息定位代码段（offset 外部可控）。
- 调试/工具链可加载未签名或可执行内容。
- 证书/profile 解析缺少对根 CA 信任链的校验。
- 降级安装路径（allowPatchDowngrade 类）无权限校验。

**历史案例**
- `app_bin_file` 配置 ownerid 豁免 → 签名保护整体失效，叠加 `LD_PRELOAD` 提权。
- ArkTS 未按签名保护信息解析 abc 在 zip 中的 offset → MAP_XPM 签名机制与云端 Verifier 双绕过。
- `ohos-arkTSScript` CLI 无权限保护，直接加载运行未签名 abc → 代码签名绕过。
- installd 调 key_enable 解析 profile 缺 pkcs7 根 CA 信任校验 → 伪造自签名 profile 注册进内核，任意 App"合法"签名构造。
- `allowPatchDowngrade` 无权限校验 → hdc shell 降级安装任意应用。

**检查点**
- 信任链每个环节（验签→解析→加载→执行）是否都有不可绕过的校验？
- 所有"豁免/特批"配置逐项上报评审。

---

## 附：历史复发热点模块清单

> 审查涉及以下模块时提高强度，并对同类历史问题做回归确认。

| 模块 | 历史问题特征 |
|---|---|
| `want_params_wrapper` / Want 解析族（ability_base） | 单文件 5+ 处空指针、递归栈耗尽、死循环、UAF；开机链路 |
| `AbilitymgrEcologicalRuleInterceptor` | 同一 Heap-use-after-free 反复 4+ 次，补丁无效 |
| `BMSBundleMultiUserInstaller` | 必现 UAF/Abrt 多环境复现 |
| CLI-SA / `claw_sandbox` / climgr 族 | 鉴权、隔离、日志、竞争全线命中（14+ 条） |
| AMS 意图/组件拉起族 | launcher-any-ability、tokenId 伪造（多条已 EXP） |
| installs / installd 文件操作原语 | 路径穿越、原语暴露、签名链校验缺失 |
| 包管理反序列化（install_param 等） | 容器大小未校验 → 内存放大/DoS |
| 各 `*_fuzzer.cpp` 测试代码 | 复制粘贴"重复使用 data"，用例无效 |

## 附：组件已知漏洞（CVE）排查提示

- 扫描第三方组件版本（zlib、Commons IO 等），比对已知 CVE；同一 CVE 需确认 ROM/SDK/各分支是否同步修复（历史上同 CVE 多分支重复提单）。
