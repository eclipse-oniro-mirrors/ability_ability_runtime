# 历史复发热点模块清单

> 审查涉及以下模块时提高强度，并对同类历史问题做回归确认。
> 来源：known-defect-patterns.md 附录。

| 模块 | 历史问题特征 |
| --- | --- |
| `want_params_wrapper` / Want 解析族（ability_base）| 单文件 5+ 处空指针、递归栈耗尽、死循环、UAF；开机链路 |
| `AbilitymgrEcologicalRuleInterceptor` | 同一 Heap-use-after-free 反复 4+ 次，补丁无效 |
| `BMSBundleMultiUserInstaller` | 必现 UAF/Abrt 多环境复现 |
| CLI-SA / `claw_sandbox` / climgr 族 | 鉴权、隔离、日志、竞争全线命中（14+ 条） |
| AMS 意图/组件拉起族 | launcher-any-ability、tokenId 伪造（多条已 EXP） |
| installs / installd 文件操作原语 | 路径穿越、原语暴露、签名链校验缺失 |
| 包管理反序列化（install_param 等） | 容器大小未校验 → 内存放大/DoS |
| 各 `*_fuzzer.cpp` 测试代码 | 复制粘贴"重复使用 data"，用例无效 |
| **want / wantagent 族（ability_base）** | **反序列化无限递归、OOB、参数伪造、DoS；WantParams::ReadFromParcel 反复 crash（20+ 条）** |
| **amsdialog** | **签名问题、弹窗劫持（悬浮窗覆盖）、调试暴露、JumpInterceptorDialog 参数注入（10+ 条）** |
| **appmgrservice / appmgr** | **进程管理权限校验缺失（JudgeSandboxByPid、GetProcessMemoryByPid 等）、后台管控绕过、死亡通知竞争（14+ 条）** |
| **abilitymgr** | **fuzz 异常、接口鉴权遗漏、生态规则拦截器 UAF（12+ 条）** |
| **uri_permission_manager** | **硬编码白名单、RawDataToStringVec/RawDataToPolicyInfo 未限制循环 → DoS（5+ 条）** |
| **ui_appearance** | **NAPI 内存泄漏（napi_create_reference、napi_open_handle_scope 异常分支）、SA_Fuzz 导致修复模式/重启（4+ 条）** |
| **form_fwk / FormMgr** | **JSON 解析未确认 array 类型、FormMgrStub 未做权限管控可 dump formid（4+ 条）** |
| **service_router_mgr** | **QueryPurposeInfos 信息泄露、StartUIExtensionAbility 转发请求绕过权限校验（3+ 条）** |
| **ability_record / lifecycle_manager** | **生命周期回调中拉起链路未管控、状态机竞争、敏感信息打印 networkid（5+ 条）** |

## 附：组件已知漏洞（CVE）排查提示

- 扫描第三方组件版本（zlib、Commons IO 等），比对已知 CVE；同一 CVE 需确认 ROM/SDK/各分支是否同步修复（历史上同 CVE 多分支重复提单）。
