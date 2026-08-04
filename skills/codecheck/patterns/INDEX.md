# 缺陷模式全量索引

> 所有 scanner 共享的"已知错误模式词典"入口。不包含执行逻辑，不定义输出格式，纯粹是"代码里长什么样的写法是错误的"。
> 每个条目格式：编号 + 模式描述 + 信号特征 + grep 线索 + 严重等级 + 历史案例引用。
>
> **新增一条历史缺陷模式的 SOP**：
> 1. 从线上 crash/bug 做根因分析
> 2. 提取信号特征（代码结构/API 调用链/语法模式）
> 3. 写 grep 线索（一个或多个搜索命令）
> 4. 按类别找到 `patterns/` 下对应 `.md` 文件，追加条目
> 5. 在本 INDEX 注册编号
> 6. 如果是高发模式（需要完整案例叙事），在 `known-defect-patterns/` 新建 `G16_xxx.md`
> 7. 引用该 pattern 文件的 scanner 下次执行时自动加载

---

## 内存安全（[memory-safety.md](memory-safety.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| MEM-001 | 空指针解引用 / 无效迭代器 | `->` 前无 nullptr 校验；`find()->second` 未判空 | `rg -n "->" --type cpp \| rg -v "nullptr\|if"` | P0 | G14 |
| MEM-002 | 数组越界 / 缓冲区写越界 | `[]`/`at()` 索引无范围校验；`memcpy` 长度失控 | `rg -n "memcpy\|strcpy\|for.*i\s*<=\s*size"` | P0 | G03 |
| MEM-003 | 释放后使用 / 生命周期 / 回调后访问 (UAF) | `delete` 后使用；异步捕获 `this` | `rg -n "delete\|lambda.*\[this\]"` | P0 | G04, G12 |
| MEM-004 | 整数溢出 / 截断 / 类型转换 | `size_t` 转小类型；unsigned 减法下溢 | `rg -n "static_cast<int>\|offset\s*\+\s*size"` | P0/P1 | G14 |
| MEM-005 | 未初始化变量 | 条件分支赋值后无条件使用 | `rg -n "int\s+\w+;"` | P0/P1 | — |

## 并发安全（[concurrency.md](concurrency.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| CONC-001 | 死锁 / 锁顺序不一致 | 相反顺序获取两把锁；持锁回调外部 | `rg -n "lock_guard\|mutex"` | P0 | — |
| CONC-002 | 竞态条件 (Check-Then-Act) | 非原子 DCL；标志位无锁读写 | `rg -n "if.*==.*nullptr"` | P0/P1 | G12 |
| CONC-003 | 数据竞争 / 共享容器无保护 | 回调容器无 mutex；sptr 跨线程 | `rg -n "vector.*callback\|shared_ptr.*_"` | P0/P1 | G12 |
| CONC-004 | 状态机竞争 / 后台管控绕过 | SIGTERM 无 SIGKILL 兜底；拉起链路无闭环检测 | `rg -n "SIGTERM\|killpg\|StartAbilityInner"` | P0/P1 | G11 |

## 输入校验与数据流（[input-validation.md](input-validation.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| INP-001 | 反序列化缺"四件套" | ReadFromParcel 无长度/深度/循环/异常兜底 | `rg -n "ReadFromParcel\|resize("` | P0 | G03 |
| INP-002 | SQL 注入 | 字符串拼接 SQL | `rg -n "ExecuteSql\|sprintf.*sql"` | P0/P1 | — |
| INP-003 | 路径遍历 / Zip Slip | 无 realpath+白名单；ZIP 未校验 `..` | `rg -n "fopen\|unzip\|\.\.\/"` | P0 | G06 |
| INP-004 | 命令注入 | 外部输入传 system/popen/exec | `rg -n "system\s*(\|popen\|execv"` | P0 | — |
| INP-005 | 格式字符串 | format 参数非字面量 | `rg -n "printf\|sprintf"` | P0/P1 | — |
| INP-006 | 日志注入 (CRLF) | 外部输入含 `\r\n` 写日志 | `rg -n "HILOG.*%\{public\}s"` | P1/P2 | — |
| INP-007 | Null Byte 注入 | std::string 含 `\0` 按 c_str() 截断 | — | P1 | — |
| INP-008 | 反序列化炸弹 / 大内存分配 | 无深度/大小限制解析；一次性分配 | `rg -n "new\s+uint8_t\[\|resize("` | P0 | — |
| INP-009 | TOCTOU | 先检查再使用（access+fopen） | `rg -n "access\s*(\|stat\s*\("` | P0/P1 | — |
| INP-010 | 协议一致性 / 序列化-反序列化不对齐 | Read/Write 字段顺序不一致 | `rg -n "ReadFromParcel\|WriteToParcel"` | P0 | G01 |

## 资源生命周期（[resource-lifecycle.md](resource-lifecycle.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| RES-001 | 资源泄漏（fd/mmap/socket/线程/callback） | 异常路径未释放 | `rg -n "fopen\|mmap\|new\s+\w"` | P1 | G04 |
| RES-002 | 析构函数资源未释放 | `~Class` 未释放成员 | `rg -n "~\w+\s*\("` | P1 | — |
| RES-003 | 虚析构函数缺失 | 基类析构非虚 + delete 派生 | `rg -n "delete\s+\w"` | P1 | — |
| RES-004 | 对象切片 | 派生按值赋给基类 | — | P1/P2 | — |
| RES-005 | 移动语义安全 | move 后未置 nullptr | `rg -n "&&\|std::move"` | P0/P1 | — |
| RES-006 | 三/五/零法则违反 | 声明其一未声明其余 | — | P1/P2 | — |
| RES-007 | 成员初始化缺失 | 成员未在初始化列表初始化 | — | P0/P1 | — |
| RES-008 | 类型转换安全 | reinterpret_cast/const_cast/iface_cast 误用 | `rg -n "reinterpret_cast\|const_cast\|iface_cast"` | P0/P1 | G14 |
| RES-009 | 进程退出时资源泄漏（误报） | 静态资源在 exit 路径未释放 | — | P3（误报） | — |
| RES-010 | 回调泄漏但数量有限（误报） | 回调总数有上限 | — | 降级（误报） | — |

## IPC 序列化（[ipc-serialization.md](ipc-serialization.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| IPC-001 | Read/Write 字段顺序不匹配 | 序列化端-反序列化端不对齐 | `rg -n "ReadFromParcel\|WriteToParcel"` | P0 | — |
| IPC-002 | 枚举值 static_cast 无范围校验 | ReadInt32 后直接强转 | `rg -n "static_cast<.*>\|ReadInt32"` | P0/P1 | — |
| IPC-003 | size 字段未校验负数 | int32 size 未校验 < 0 即 resize | `rg -n "ReadInt32\|resize("` | P0 | G03 |
| IPC-004 | fd 跨 IPC 边界所有权 | 未 dup；异常路径未 close | `rg -n "WriteFileDescriptor\|ReadFileDescriptor\|dup"` | P1 | — |
| IPC-005 | 校验位置错（位置错） | 鉴权做在客户端；只覆盖第一跳 | `rg -n "OnRemoteRequest\|CheckPermission"` | P0 | G02 |
| IPC-006 | 身份来源错（身份错） | 信任 Parcel 自报身份；pid 校验 | `rg -n "GetCallingPid\|data.ReadString"` | P0 | G01 |
| IPC-007 | 调用链中转错（链路错） | SA 中继未二次授权；ResetCallingIdentity 未恢复 | `rg -n "ResetCallingIdentity\|isProxy"` | P0 | — |
| IPC-008 | 分发与解析错（分发错） | default 有副作用；校验失败未 return | `rg -n "default:\|return\s+true"` | P0 | G14 |
| IPC-009 | 校验逻辑缺陷（逻辑错） | 黑名单；前缀匹配；只校验能不能调 | `rg -n "find\(\|starts_with"` | P0 | — |
| IPC-010 | 跨设备分布式身份采信（状态错） | 直接采信对端 userId/包名 | `rg -n "distributed\|ContinueMission"` | P0/P1 | — |

## 权限与鉴权（[privilege-auth.md](privilege-auth.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| AUTH-001 | 凭据管理缺陷 | 明文存储 Token/Key；不安全算法 | `rg -n "password\|secret\|token\s*=" -i` | P0 | G07 |
| AUTH-002 | 日志红线关键字 | 打印 udid/networkid/SN/challenge/token/cmdLine | `rg -n "udid\|networkid\|SN\|challenge\|token\|cmdLine"` | P0 | G07 |
| AUTH-003 | 权限管控不足 | 仅 BundleName 白名单；升级路径权限丢失 | `rg -n "bundleName\|readPermission\|writePermission"` | P0/P1 | G08 |
| AUTH-004 | 环境残留与隐藏面 | user 版本含调试入口；命令未文档化 | `rg -n "getopt\|argc\|argv"` | P0/P1 | G05 |
| AUTH-005 | SELinux 与文件权限 | db 标签过大；CLI 无独立标签 | — | P0/P1 | G08 |
| AUTH-006 | 系统公共事件与隐私指示器 | 公共事件未加入列表；隐私指示器未判返回值 | `rg -n "CommonEvent\|PrivacyIndicator"` | P1 | — |
| AUTH-007 | 硬编码白名单（机制设计缺陷） | 硬编码包名白名单；resv 字段未 reset | `rg -n "\"com\.ohos\.\w+\"\|resv"` | P0/P1 | G13 |
| AUTH-008 | 签名/安装管控绕过 | ownerid 豁免；abc offset 外部可控 | — | P0 | G10 |

## 错误处理（[error-handling.md](error-handling.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| ERR-001 | 忽略返回值 | ReadString/Insert/WriteToFile 返回值未检查 | `rg -n "parcel\.Read\|Unmarshalling\("` | P0/P1 | G14 |
| ERR-002 | 错误路径返回成功 | 异常分支返回 true/ERR_OK | `rg -n "return\s+true\|return\s+ERR_OK"` | P0/P1 | G14 |
| ERR-003 | 类型转换无异常兜底 | std::stoi 无 try-catch | `rg -n "std::stoi\|std::stol"` | P0 | G03 |
| ERR-004 | 部分失败 / 未回滚 | isOpen_ 提前置 true；多表无原子性 | `rg -n "isOpen_\s*=\s*true"` | P0/P1 | — |
| ERR-005 | 错误码语义不一致 | 文档说 X 实现返回 Y；错误码比较写反 | `rg -n "return\s+ERR_\|==\s*0"` | P1 | — |
| ERR-006 | 异常跨边界 | 异常跨越 Native/IPC 边界 | `rg -n "throw"` | P0 | — |
| ERR-007 | 防御性 double-free（误报） | free(ptr); ptr=nullptr; 再 free | — | 推翻（误报） | — |
| ERR-008 | UAF 仅涉及日志打印（误报） | 释放后使用仅在 HILOG_INFO 读整数 | — | 降两级（误报） | — |
| ERR-009 | 静态 bool 标志位无锁（误报） | 一次性初始化标志位 | — | P3（误报） | — |

## 逻辑正确性（[logic-correctness.md](logic-correctness.md)）

| 编号 | 模式描述 | 信号特征 | grep 线索 | 严重等级 | 历史案例 |
|------|---------|---------|-----------|---------|---------|
| LOG-001 | 死代码 | 永不为真条件；return 后代码 | — | P2/P3 | — |
| LOG-002 | 逻辑矛盾 | 互斥条件同时为真 | — | P1/P2 | — |
| LOG-003 | 条件覆盖不完整 | 枚举缺分支；switch 缺 default | — | P1 | — |
| LOG-004 | 数据污染 | 外部数据未验证传到 sink | — | P0 | — |
| LOG-005 | 类型不匹配 | 有符号/无符号比较；枚举混用 | — | P0/P1 | — |
| LOG-006 | 非法状态转换 | 跳过中间状态无校验 | — | P0/P1 | — |
| LOG-007 | 状态不一致 | 多状态变量不同步 | — | P1 | — |
| LOG-008 | 状态机死锁 | 无法到达终态 | — | P0/P1 | — |
| LOG-009 | 遗漏错误路径 | 不检查返回值；未回滚 | — | P1 | — |
| LOG-010 | 不变性破坏 | 操作后未维护约束 | — | P0/P1 | — |
| LOG-011 | 契约违反 | 前置/后置条件未验证 | — | P1 | — |
| LOG-012 | 整数溢出（逻辑层） | 算术运算溢出 | — | P0/P1 | — |

## API 一致性反模式（[api-consistency.md](api-consistency.md)）

| 编号 | 模式描述 | 信号特征 | 严重等级 | 历史案例 |
|------|---------|---------|---------|---------|
| API-001 | 资料×接口定义×实现 签名不一致 | docs×d.ts×cpp 方法名/参数/类型不符 | P1 | — |
| API-002 | 起始版本不一致（最高频出错点） | docs×d.ts@since×ndk.json 数值不符 | P1 | — |
| API-003 | 错误码不一致 | docs 错误码表×d.ts@throws×实现 不符 | P1 | — |
| API-004 | 参数校验与接口定义不一致 | 空指针/非法值未返回 401 | P0/P1 | — |
| API-005 | 资料 E 项：描述与实现行为不一致 | docs 描述 A 代码实现 B（E1-E7） | P1/P0(E4) | — |
| API-006 | 资料存在性与内容完整性 | d.ts 声明但 docs 缺章节 | P0 | — |
| API-007 | 框架实现内部 bug | 内存配对/溢出/浅拷贝/拼写 | P0/P1 | — |
| API-008 | 服务侧生命周期/线程安全 | lambda 捕获栈引用；RAII guard 析构副作用 | P0/P1 | — |
| API-009 | 服务侧资源泄漏 | 异常路径 fd/内存/锁/回调未清理 | P0/P1 | — |
| API-010 | 服务侧 IPC 序列化 | Read/Write 顺序不匹配；枚举无范围校验 | P0/P1 | — |
| API-011 | 服务侧 API 行为/副作用 | 隐含限制未声明；权限校验被绕过 | P0/P1 | — |
| API-012 | 框架实现文件静态/动态混淆 | 把 ets_/cj_ 当 js_ 扫 | — | — |
| API-013 | BUILD.gn relative_install_dir 区分新旧同名模块 | 新旧 NAPI 模块目录名仅差一层级 | — | — |

## 已知缺陷模式（[known-defect-patterns/](known-defect-patterns/)）

| 编号 | 模式 | 文件 |
|------|------|------|
| G01 | 身份信任错误 | [G01_identity_trust.md](known-defect-patterns/G01_identity_trust.md) |
| G02 | 鉴权遗漏分支 | [G02_auth_missing_branch.md](known-defect-patterns/G02_auth_missing_branch.md) |
| G03 | 反序列化缺"四件套" | [G03_deserialization_quartet.md](known-defect-patterns/G03_deserialization_quartet.md) |
| G04 | 同族 API 模式化误用 | [G04_api_family_misuse.md](known-defect-patterns/G04_api_family_misuse.md) |
| G05 | user 版本隐藏面 | [G05_release_hidden_surface.md](known-defect-patterns/G05_release_hidden_surface.md) |
| G06 | 路径穿越与 Zip Slip | [G06_path_traversal_zip_slip.md](known-defect-patterns/G06_path_traversal_zip_slip.md) |
| G07 | 日志红线 | [G07_log_redline.md](known-defect-patterns/G07_log_redline.md) |
| G08 | 配置级提权 | [G08_config_privilege_escalation.md](known-defect-patterns/G08_config_privilege_escalation.md) |
| G09 | 沙箱隔离失效 | [G09_sandbox_isolation_failure.md](known-defect-patterns/G09_sandbox_isolation_failure.md) |
| G10 | 签名/安装管控绕过 | [G10_signature_install_bypass.md](known-defect-patterns/G10_signature_install_bypass.md) |
| G11 | 生命周期状态机与后台拉起管控 | [G11_lifecycle_state_machine.md](known-defect-patterns/G11_lifecycle_state_machine.md) |
| G12 | 条件竞争 | [G12_race_condition.md](known-defect-patterns/G12_race_condition.md) |
| G13 | 机制设计缺陷 | [G13_design_defect.md](known-defect-patterns/G13_design_defect.md) |
| G14 | 返回值与异常分支处理 | [G14_return_value_exception.md](known-defect-patterns/G14_return_value_exception.md) |
| G15 | UIExtension / 窗口组件暴露面 | [G15_uiextension_exposure.md](known-defect-patterns/G15_uiextension_exposure.md) |
| — | 历史复发热点模块清单 | [hotspot-modules.md](known-defect-patterns/hotspot-modules.md) |
