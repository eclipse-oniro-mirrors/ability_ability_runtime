---
name: security-scanner
description: >
  合并版 security scanner：高影响缺陷审计 + 商用前安全审查。
  按最终结果影响（崩溃/挂死/OOM/UAF/死锁/数据损坏/权限绕过/敏感数据泄漏）优先级排序，
  并对命中历史缺陷模式（G1-G16）做全库同类横扫。
  Use when asked to find, review, scan, investigate, or prioritize P0/P1 risks
  (crashes, hangs, deadlocks, OOM, illegal memory access, data corruption,
  destructive side effects, resource leaks, state pollution, permission bypass,
  serialization/IPC bugs, malformed input handling, file/path/archive parsing bugs,
  lifecycle bugs, concurrency races), OR when the user wants 安全审查、安全审计、
  漏洞扫描、商用前 review、内存/权限/敏感数据/IPC 鉴权专项排查。
---

# Security Scanner — 高影响缺陷 + 安全审查合并扫描器

## 定位

本 scanner 将高影响缺陷审计与商用前安全审查合并为一个 scanner，消除 7 项重复扫描（空指针、越界、UAF、整数溢出、死锁、反序列化），统一执行一次 Core Scan，同时输出 bug 视角与 security 视角的双标注。

**Pattern 引用声明**：本 scanner 加载以下 pattern 文件作为检查规则来源：
- [`patterns/memory-safety.md`](../../patterns/memory-safety.md) — MEM-001~005
- [`patterns/concurrency.md`](../../patterns/concurrency.md) — CONC-001~004
- [`patterns/input-validation.md`](../../patterns/input-validation.md) — INP-001~010
- [`patterns/resource-lifecycle.md`](../../patterns/resource-lifecycle.md) — RES-001~010
- [`patterns/ipc-serialization.md`](../../patterns/ipc-serialization.md) — IPC-001~010
- [`patterns/privilege-auth.md`](../../patterns/privilege-auth.md) — AUTH-001~008
- [`patterns/error-handling.md`](../../patterns/error-handling.md) — ERR-001~009
- [`patterns/known-defect-patterns/`](../../patterns/known-defect-patterns/) — G01~G16 + hotspot-modules

---

> **Path B 声明**：本 scanner 直接被调用时，除产出下述原始发现清单（发现模板 + Excel）外，必须按 [`conventions.md`](../../conventions.md) §5–§13 规程生成符合 `codecheck_report_TEMPLATE.md` 的统一报告。严重等级五档（9-10/7-8/5-6/3-4/1-2）按 conventions §7.2 归一化到 P0–P3。P0 项必须做 refute（见 conventions §13.2）。

## 工作流

### Step 1: 模块风险画像 + 入口面登记

**模块风险画像**：
- 模块主要能力（一句话）
- 外部输入点（公共 API / IPC / 文件 / 配置）
- 资源所有权（fd / mmap / socket / 线程 / callback / native ref）
- 副作用（路径写入/删除、进程拉起、广播、状态变更）
- 异步/并发行为
- 可能的 P0/P1 结果影响

**入口面登记**（安全审查维度）：
- IPC Stub / `OnRemoteRequest` / `OnRemoteRequestEx`
- Parcel / JSON / 二进制反序列化
- CLI 参数（`argc`/`argv`/`getopt`）
- 文件/路径输入
- 网络与配置数据
- NAPI 接口参数（`napi_get_value_*`）

> **完成标准**：模块能力、外部输入点、资源所有权、并发行为均已列出。

### Step 2: Core Scan（统一扫一次）

加载 `patterns/` 下所有相关文件，执行单次遍历。检查项（共 11 项）：

| # | 检查项 | Pattern 文件 |
|---|--------|-------------|
| ① | 空指针/无效迭代器 | `patterns/memory-safety.md` MEM-001 |
| ② | 越界访问 | `patterns/memory-safety.md` MEM-002 |
| ③ | UAF/生命周期/回调后访问 | `patterns/memory-safety.md` MEM-003 |
| ④ | 整数溢出/截断/类型转换 | `patterns/memory-safety.md` MEM-004 |
| ⑤ | 死锁/竞态/数据竞争 | `patterns/concurrency.md` CONC-001~004 |
| ⑥ | 资源泄漏（fd/mmap/socket/callback/锁） | `patterns/resource-lifecycle.md` RES-001~008 |
| ⑦ | 反序列化四件套 | `patterns/input-validation.md` INP-001 |
| ⑧ | 敏感信息泄漏/日志红线 | `patterns/privilege-auth.md` AUTH-001~002 |
| ⑨ | 系统框架合规/环境残留 | `patterns/privilege-auth.md` AUTH-003~006 |
| ⑩ | 类安全/虚析构/移动语义 | `patterns/resource-lifecycle.md` RES-003~008 |
| ⑪ | IPC 鉴权（条件启用） | `patterns/ipc-serialization.md` IPC-005~010 + `patterns/privilege-auth.md` AUTH-007~008 |

> ⑪ 启用条件：当代码涉及 `IRemoteObject`/Stub/Proxy/`IPCSkeleton`/`OnRemoteRequest`/`SendRequest`/`WriteRemoteObject`/`GetSystemAbility` 时启用。

> **完成标准**：11 项每一条均已给出结论（命中 / 不适用），不得跳过未判。

### Step 3: 已知缺陷模式同类横扫（G 类）

加载 `patterns/known-defect-patterns/` 全部文件。对命中模式做全库 grep，列出同类写法。

涉及热点模块（见 `patterns/known-defect-patterns/hotspot-modules.md`）时提高审查强度，并对历史问题做回归确认。

> **同类横扫是本 scanner 的核心行为**：发现一处即追问"库中还有没有同样写法"，禁止只报单点。这是 predictability 引擎——每一次命中都要把同类挖尽。
>
> **完成标准**：命中模式的同类写法已全部列出，或已声明全库搜索确认无其他同类点。

### Step 4: 双视角标注 + 证据分级

对每个发现同时标注两个视角（不互斥）：

**bug 视角**：
- 证据分级：Confirmed / Likely / Suspicious / Excluded
- Priority = 影响严重程度 × 可触发性 × 影响范围（波及面）
  - P0：进程崩溃、卡死、死锁、非法内存访问、OOM、未捕获 Native 异常 → 立即修复
  - P1：数据损坏、破坏性副作用、权限绕过、持续性资源泄漏、状态污染 → 高优先级
  - P2：功能失败、错误结果、可恢复边界错误 → 计划修复
  - P3：可观测性、可维护性 → 暂缓

**security 视角**：
- 所属审计维度：A（内存/执行）/ B（输入校验）/ C（敏感信息）/ D（系统框架）/ E（类与对象）/ F（IPC 鉴权）
- G 类编号（如命中 G01-G16）
- 同类清单（全库 grep 结果）

> 一个发现可以同时是高优先级崩溃风险和安全攻击面。

### Step 5: 产出发现列表

按模块一级分组，模块内按严重度降序（10→1 五档）输出。

**Priority 公式**：
```text
Priority = 影响严重程度 * 可触发性 * 影响范围
```

**证据等级**：

| 等级 | 含义 | 输出处理 |
|------|------|---------|
| Confirmed | 有明确代码证据和可触发路径 | 进入正式发现，给出修复和测试建议 |
| Likely | 代码风险明确，触发路径还需确认 | 作为高优先级候选跟进 |
| Suspicious | 模式可疑，结果或触发性不明确 | 放入 follow-up，不阻塞主线 |
| Excluded | 已确认被上游条件/不变量/调用约束保护 | 记录排除理由，避免重复排查 |

---

## 输出格式

### 发现模板（每条 Confirmed/Likely 项）

```text
标题：
位置：file:line（多处调用全部列出）
优先级：P0/P1/P2/P3
证据等级：Confirmed/Likely/Suspicious
bug 视角：
  结果影响：
  触发条件：
  影响范围：
  根本原因：
security 视角：
  审计维度：A/B/C/D/E/F
  G 类编号：（如命中）
同类排查结果：（命中 G 时必填）全库同类写法 file:line 清单
证据：
推荐修复：
推荐测试：
```

触发路径应具体：
```text
通过 API Y，给定输入 X，代码路径 Z 可导致结果 R。
```

### Excel 输出（当请求 Excel 时）

| 列 | 表头 | 说明 |
|----|------|------|
| A | 文件路径 | 相对路径 |
| B | 行号 | 如 `41`、`157-158` |
| C | 问题概述 | 一句话（≤50 字符） |
| D | 问题详细描述 | `### 问题描述` + `### 修复建议` + `### 影响` |
| E | 问题类型 | 内存安全/整数安全/并发安全/输入验证/状态污染/资源泄漏/逻辑缺陷/初始化安全/错误处理/封装违反/信息泄漏/权限安全 |
| F | 风险等级 | 致命/严重/一般/提示 |

样式：表头深蓝底（`4472C4`）白字加粗；数据行 Calibri 11pt 自动换行垂直顶对齐；细边框；列宽 A=55 B=12 C=45 D=90 E=15 F=10；冻结首行；按 风险等级降序后文件路径排序。

### 统一报告（直接被调用时）

直接被调用时，按 [`conventions.md`](../../conventions.md) §5–§13 生成符合 `codecheck_report_TEMPLATE.md` 的统一报告，不再使用本 scanner 自有"安全审计报告"骨架。五档严重度（9-10/7-8/5-6/3-4/1-2）按 §7.2 归一化到 P0–P3 后进入统一报告。本节"发现模板"与"Excel 输出"作为原始发现清单单独交付（见 [`orchestrator/SKILL.md`](../../orchestrator/SKILL.md) Step 7 交付项），不归档入统一报告正文。

---

## 守则

- **中文报告**：统一报告与原始发现清单全文中文。
- **修复方式审查**：对已有补丁、特判、豁免保持怀疑——验证是"机制性根治"还是"点位封堵"，后者按 G6 上报绕过风险。
- **不动代码**：本阶段只产报告与建议，不直接改源码（修复另起任务）。
- **全量覆盖**：A-F 每一条均已给出结论，不得跳过未判。
- **≥80 字推演**：每条发现含 ≥80 字技术推演 + 代码位置 + 同类清单（如命中 G）+ 修复建议。
- **按优先级排序**，而非按文件顺序。
- **在可触发性被证伪之前，将 P0/P1 候选视为高优先级**；仅在结果影响或可触发性明显有限时才降级。
- **优先少而精**：宁少而证据充分，勿多而代码气味充数。
