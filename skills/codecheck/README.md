# Codecheck V2：自适应代码检视 Skill

入口：[`SKILL.md`](SKILL.md)。核心是**理解行为 → 自主发现 → 按风险挑战 → 独立证伪 → 证据交付**。不再将检视委派给固定的 Security/Logic/Input/API Scanner，也不在 Skill 内维护 Patterns；不要求每次进行相同深度或轮数的攻防。

## 工作机制

```text
审查范围（diff / commit / PR / path）
              │
         Understand
       Concern / 调用链 / 状态
              │
          Discover  ──────── 按需检索外部知识
       自主检视行为与契约      standards / specifications
              │
    Challenge（按风险深入）
       反例推导 / 可控验证
              │
           Refute
       触发链核验 / 证伪 / 去重
              │
           Integrate
         报告 / trace / 门禁
```

## 目录与职责

- `SKILL.md`：唯一新的执行入口、阶段契约与停止条件。
- `references/review-workflow.md`：按 Concern 组织上下文与基础检视。
- `references/knowledge-retrieval.md`：从独立 [abilitykit_sdd](https://gitcode.com/abilitykit_public/abilitykit_sdd) 仓库按需检索 `standards/`、`specifications/`；知识不在本仓复制。
- `references/adversarial-review.md`：自主攻防与反事实验证。
- `references/refutation-protocol.md`：独立 Refuter，重新判断真实风险。
- `references/report-contract.md`：报告、审计 trace 与旧门禁兼容。
- `codecheck_report_TEMPLATE.md`：17 字段机器元数据（schema 2.0）、决策与章节格式。
- `conventions.md`：版本、等级、检视能力维度与计分决策的唯一说明。
- `scripts/kb_lookup.py`：只读本地知识库候选检索（路径/版本/行号，需二次确认）。
- `scripts/validate_report.py`：校验门禁元数据及 V2 trace 基本一致性。
- `evals/`：真实历史缺陷回放、消融测试与人工裁决规程。

## 与旧版的区别

| 方面 | V1 | V2 |
|---|---|---|
| 任务组织 | 固定 Scanner + 路径路由 | 行为 Concern / 跨 Concern 交互 |
| 知识来源 | 内置 Patterns / G 类规则 | 外部 standards / specifications 按需引用；历史问题可存外部案例库 |
| 发现方式 | 按规则类型探索 | 从不变量、代码行为与上下文生成检视假设 |
| 攻防 | Refute 现有发现 | 先主动构造新反例，再独立 Refute |
| 误报控制 | 按严重性抽样复核 | 所有正式候选均核对触发、影响与归因 |
| 报告 | Finding + 旧门禁 | 同样的旧门禁 + Concern/Knowledge/Challenge/Coverage trace |

## 报告契约（schema 2.0）

机器元数据沿用历史模板的 17 个字段名与评分权重（30/12/5/2），但 `schema_version` 升为 **`2.0`**，三处契约按 V2 重新定义，**不与 v1 互认**：

- `dimensions_required` / `dimensions_executed` 取值域改为 V2 能力名 `behavior` / `security` / `input` / `api`（`behavior`、`security` 为基线必检），是对实际检视覆盖的声明，不表示运行过任何 Scanner。
- 决策矩阵按 V2 判定：覆盖不足优先记 `insufficient`（即使已有 P0）；累计扣分使 score < 70 时 `block`。映射规则见 [`conventions.md`](conventions.md)。
- 报告正文不含 Scanner 列或扫描器标识。

机器 YAML 不新增 V2 字段；更丰富的 Concern、假设、规格引用与覆盖数据写入 `review_trace_*.json`。旧版 schema 1.0 报告只作存档对照，不再作为门禁输入；依赖旧字段语义的消费端须按上述取值域与矩阵自行适配。

## 运行建议

- 默认只读源码；外部知识库优先使用已检出的本地副本，可通过 `CODECHECK_KB_ROOT=/path/to/abilitykit_sdd` 指定。
- 查不到/读不到相关规格时记录缺口；必要时门禁 `insufficient`，不得无来源断言规范要求。
- 不以未发现 Finding 证明代码“无问题”；未覆盖的 Concern 与未证实的假设必须可追溯。
- `Challenge` 按代码的真实风险和新增证据价值决定深度；`done` 要有调查记录，`not_applicable` 要有理由，受限则记录 `limited`。不设反例数量或固定攻防轮数。
- 上库前优先使用 `evals/` 的真实缺陷基线与旧版对照，测量新增有效发现与误报，而非只比较报告篇幅。
