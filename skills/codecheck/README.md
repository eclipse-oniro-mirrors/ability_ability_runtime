# codecheck — 代码检视技能集

本目录是 ability_runtime 仓库的**代码检视工作台**。当用户说"检视一下代码""帮我审一下""做一次 code review""深度扫描"时，从这里出发：先读本 README 了解三层架构，由 orchestrator 自动探测路径特征、选择 scanner 组合、并行调度、Refute 对抗性验证、合并为一份统一报告。

> 入口指引：`AGENTS.md` 的「代码检视」章节把"检视代码"这一意图路由到本 README。本 README 只负责**导航与编排**，具体方法论在各层文件中。

---

## 三层架构

核心原则：**知识（Pattern）、执行（Scanner）、调度（Orchestrator）三层分离**，增加对抗性验证层（Refuter）。

```
skills/codecheck/
│
├── README.md                              # 总入口（本文件）
├── codecheck_report_TEMPLATE.md           # 统一报告模板（门禁合约）
├── conventions.md                         # 跨模块公约
│
├── patterns/                              # 第一层：中央缺陷模式知识库
│   ├── INDEX.md                           #   全量模式索引
│   ├── memory-safety.md                   #   内存安全（MEM-001~005）
│   ├── concurrency.md                     #   并发安全（CONC-001~004）
│   ├── input-validation.md                #   输入校验与数据流（INP-001~010）
│   ├── resource-lifecycle.md              #   资源生命周期（RES-001~010）
│   ├── ipc-serialization.md               #   IPC 序列化（IPC-001~010）
│   ├── privilege-auth.md                  #   权限与鉴权（AUTH-001~008）
│   ├── error-handling.md                  #   错误处理（ERR-001~009）
│   ├── logic-correctness.md               #   逻辑正确性（LOG-001~012）
│   ├── api-consistency.md                 #   API 一致性反模式（API-001~013）
│   └── known-defect-patterns/             #   G01–G15 历史缺陷模式库
│       ├── G01_xxx.md ... G15_xxx.md       #   每模式一文件：信号特征 + grep 线索 + 历史案例 + 检查点
│       └── hotspot-modules.md             #   热区模块清单
│
├── scanners/                              # 第二层：检视扫描器（4 个薄层）
│   ├── security-scanner/SKILL.md          #   高影响缺陷 + 安全审查（合并版）
│   ├── logic-scanner/SKILL.md             #   逻辑变更分析
│   ├── input-scanner/SKILL.md             #   外部输入全链路审计
│   └── api-scanner/SKILL.md               #   API 一致性审计
│
└── orchestrator/                          # 第三层：调度与报告
    ├── SKILL.md                           #   唯一编排器（调度 + refute + 合并报告）
    └── refute-rules.md                    #   Refuter 质疑规则与判定标准
```

### 三层职责

| 层 | 职责 | 不包含 |
|----|------|--------|
| **patterns/** | 所有 scanner 共享的"已知错误模式词典"。每条目：信号特征 + grep 线索 + 触发条件 + 典型后果 + 历史案例引用 | 不包含执行逻辑、不定义输出格式 |
| **scanners/** | 每个 scanner 的 SKILL.md 只保留三部分：执行工作流 + 输出格式定义 + Pattern 引用声明（200-400 行） | 不含具体检查规则、grep 命令、代码模式（这些在 patterns/） |
| **orchestrator/** | 唯一编排器：自动探测 + 选择 scanner 组合 + 并行调度 + Refute 对抗性验证 + 合并统一报告 | 不执行具体扫描 |

---

## Scanner 一览

| Scanner | 检视维度 | 触发场景 | 输出 |
|---------|---------|---------|------|
| [`security-scanner`](scanners/security-scanner/SKILL.md) | 高影响缺陷 + 商用前安全审查（合并版）：崩溃、挂死、OOM、UAF、死锁、数据损坏、资源泄漏、状态污染、权限绕过、敏感数据泄漏、IPC 鉴权 | "查高危 bug""P0/P1 风险排查""崩溃/挂死审计""安全审查""漏洞扫描""商用前 review" | 按 `影响×可触发性×波及面` 排序的双视角（bug+security）缺陷清单 + G 类同类横扫 |
| [`logic-scanner`](scanners/logic-scanner/SKILL.md) | 逻辑影响：修改的波及路径、逻辑一致性、状态机转换、边界条件、错误处理 | "逻辑分析""这段改动会影响什么""状态机/数据流/控制流检查" | 逻辑影响范围 + 不一致/边界遗漏清单 |
| [`input-scanner`](scanners/input-scanner/SKILL.md) | "外部输入 → 持久化"全链路健壮性：IPC/HTTP/CLI/配置/网络 → DB/文件/缓存/日志 | "外部输入排查""输入接口审计""持久化安全检查""接口健壮性" | 按 P0/P1/P2 排序的 Excel 风险清单（写入侧+读取侧双维度） |
| [`api-scanner`](scanners/api-scanner/SKILL.md) | 对外 API 全量一致性：资料文档×接口定义×框架实现×测试用例完备度，三轮扫描 | "接口审计""API 一致性""测试用例完备度""扫一下 xxxKit" | `<kit>_api_audit.md` + `<kit>_api_audit.csv` 双格式 |

> **Path B 统一合约**：任一 scanner 直接被调用时，除上表"输出"列的原始发现清单外，必须按 [`conventions.md`](conventions.md) §5–§13 额外生成符合 [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md) 的统一报告（双产出）。经 orchestrator 调度时只产原始发现清单，由 orchestrator 合并生成统一报告。

---

## 维度关系图

```
              用户："检视一下代码" / "深度扫描"
                           │
                           ▼
               ┌─────────────────────────┐
               │      orchestrator       │  唯一编排：自动探测 + 选择组合 + refute + 合并报告
               └────────────┬─────────────┘
                            │ 并行调度（按路径特征选）
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
  │  security    │  │   logic      │  │   input      │  │     api      │
  │  -scanner    │  │  -scanner    │  │  -scanner    │  │  -scanner    │
  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘
         │                 │                 │                 │
         └────────┬────────┴────────┬────────┘                 │
                  ▼                 ▼                           │
            patterns/         patterns/                   patterns/
            (memory-safety,    (logic-correctness,         (api-consistency,
             concurrency,       error-handling)              ...)
             input-validation,
             resource-lifecycle,
             ipc-serialization,
             privilege-auth,
             known-defect-patterns/G01-G15)
```

> - `orchestrator` 是**唯一编排器**，自动探测路径特征（IPC/持久化/API 信号）后选择 scanner 组合，并行调度，Refute 验证，合并统一报告。通用"检视代码"/"深度扫描"的默认入口。
> - 单一维度（纯安全/纯 API/纯外部输入/纯逻辑）时直接调对应 scanner，不走 orchestrator。**直接调用时 scanner 也必须按 [`conventions.md`](conventions.md) §5–§13 生成统一报告**（双产出：原始发现清单 + 统一报告）。
> - 所有 scanner 共享 `patterns/` 知识库，避免重复维护检查规则。

---

## 检视流程（"检视一下代码"时怎么走）

### Step 1：界定范围
明确两点，缺则向用户确认：
1. **目标路径或 Kit**：如 `services/abilitymgr/src/`、`frameworks/native/ability/native/`、或 `abilityKit`。
2. **检视重点**：通用 review / 安全 / 高危 bug / API 兼容 / 外部输入健壮性。未指定时走"通用检视"。

### Step 2：自动探测 + 选择 scanner 组合

orchestrator 自动探测路径特征：

| 探测信号 | grep 线索 |
|---------|----------|
| IPC 信号 | `rg "Parcel::Read\|OnRemoteRequest\|WriteRemoteObject" --type cpp` |
| 持久化信号 | `rg "Insert\s*(\|ExecuteSql\|fopen\|write\s*\(" --type cpp` |
| API 信号 | 检查 `interfaces/kits/` 或 `frameworks/js/napi/` 下是否有 `.d.ts`/`.h` |

按特征 + 用户意图决策：

| 条件 | 调用组合 |
|------|---------|
| 用户指定单一维度 → | 只调对应 scanner，跳过推断 |
| `services/` + IPC 信号 + 持久化信号 → | security + logic + input |
| `interfaces/` 或 `frameworks/js/napi/` 且存在 `.d.ts` → | security + logic + api |
| 通用路径无特殊信号 → | security + logic |
| 用户说"全面"/"最大覆盖" → | security + logic + input + api |

### Step 3：并行调度 scanner
读对应目录的 `SKILL.md`，按其工作流执行。每个 scanner 加载 `patterns/` 下相关 pattern 文件。保留各 scanner 原始产出（md/csv/excel），不要在中间改写。

### Step 4：Refute — 对抗性验证 🔥 新增
orchestrator 对每条发现执行三层质疑：
1. **触发路径证伪**：上游有防护吗？真的可达吗？
2. **影响夸大证伪**：后果真的这么严重吗？
3. **跨 scanner 根因去重**：两条发现本质同一个根因吗？

产出 `refute_log.md`，记录每条发现的判定：✅ 维持 | ⬇️ 降级 | ❌ 推翻 | 🔀 合并。详见 [`orchestrator/refute-rules.md`](orchestrator/refute-rules.md)。

### Step 5：合并为统一报告
基于 `refute_log.md` 过滤后的发现列表，按 [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md) 生成 `codecheck_report_<scope>_<YYYYMMDD>.md`。

> 📄 **权威模板：** [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md)
> 模板固定了章节顺序（门禁结论 → 扣分原因 → 必须立即处理 → 建议跟进 → 分维度速览 → 关键发现详情）与头部机器可读的 YAML 报告元数据块；评分公式与门禁决策矩阵为通用规则，见 [`conventions.md`](conventions.md) §9，不在输出报告中呈现。所有 codecheck 报告（含 orchestrator 合并、单 scanner 直接产出）格式保持一致，便于门禁脚本解析与历史对比。

生成要求（门禁校验项）：
1. 头部 YAML 报告元数据块字段名/取值域固定，`score`/`risk_level`/`gate_decision` 必须按 [`conventions.md`](conventions.md) §9 的公式与决策矩阵计算，不得自创分值。
2. 严重等级统一归一化为 P0–P3（各 scanner 的 critical/high/medium/low、致命/严重/一般/提示 一律映射后汇总，映射表见 [`conventions.md`](conventions.md)）。
3. 跨维度去重：同一 `file:line` 被多个 scanner 命中时在"必须立即处理"节合并为一条，标注维度来源列表。不同 `file:line` 但同根因由 refuter 合并。
4. 必检维度缺失时，门禁决策为 `insufficient`（无法评估），需补齐扫描后重出报告。

### Step 6：交付
交付统一报告路径 + Executive Summary + Top 高危项 + 各 scanner 原始产出路径 + `refute_log.md` 路径。

---

## 约定

跨模块公约集中管理在 [`conventions.md`](conventions.md)，要点：

- **静态语言实现默认排除**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp` 等 static 侧不在扫描范围，除非用户明确要求包含。
- **证据要求**：每条发现必须可追溯到 `file:line` + 触发路径，不收"代码气味"级别的无证据项。
- **不动代码**：检视阶段只产出报告与建议，不直接改源码；修复由用户确认后另起任务。
- **去重规则**：跨 scanner 去重以 `file:line` 为第一键；同位置多 scanner 命中合并为一条，标注维度来源；不同 `file:line` 但同根因由 refuter 合并。
- **Refute 公约**：只质疑不发现，质疑必须有代码证据，被推翻发现保留在附录，P0 推翻需"确认不可达"级证据。
