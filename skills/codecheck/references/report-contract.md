# 报告与门禁契约：机器元数据 + V2 真实检视记录

## 必交付三件套

1. `codecheck_report_<scope>_<YYYYMMDD>.md` — 给人工阅读和门禁消费。严格遵循 `../codecheck_report_TEMPLATE.md` 的 **17 字段机器元数据（schema 2.0）**、章节顺序、严重等级、评分与门禁。
2. `review_trace_<scope>_<YYYYMMDD>.json` — V2 的真实执行、Concern 覆盖、动态检索引用、挑战、限制和每条裁决。
3. `refute_log_<scope>_<YYYYMMDD>.md` — 独立证伪记录（包括推翻、未决、合并项）。

命名中的 `scope` 应转为安全的短标识（建议 `[a-zA-Z0-9._-]`），避免将路径分隔符带入输出文件名。

## 机器元数据的含义

字段名与 17 字段结构沿用历史模板，`schema_version` 为 `2.0`；评分公式与决策矩阵以 `../conventions.md` 为准（V2 定义）。`dimensions_*` 记录 V2 检视能力（`behavior` / `security` / `input` / `api`）经事实核实的实际覆盖，不代表运行任何 Scanner；V2 的真实执行阶段放在 trace，不进入机器元数据。不能仅因为进入过某个阶段就标记该能力完成。

**重要**：报告不声明任何 Scanner 运行记录，也不伪造 Scanner 原始产物。如外部消费端仍依赖旧 Scanner 产物或 schema 1.0 报告，须按 V2 trace 与新取值域自行适配。

当关键 Concern `limited`、外部必要契约不可访问、核心验证步骤无法完成而影响合入判断时，把 `dimensions_executed` 留下诚实缺口并赋 `insufficient`；即使存在已确认 P0，机器决策按矩阵仍先记 `insufficient`，但在第一节显著标明已知阻塞风险，不能解释为“没有 P0”。

## 报告规则

- 仅经 Refuter 判 `confirmed` 且有触发链证据的候选进入 Finding 总数。未决项列“建议跟进/未决风险”但**不计作 P3 或正式 finding**。
- P0/P1 必须可见、可复核；P2/P3 必须确有证据，不允许用不确定性代替严重性。
- 发现条目保留 `file:line`、触发条件、变更归因、影响、代码证据、规格引用（若真实适用）、修复目标、测试状态。对于外部合同争议保留来源 SHA。
- 对同根因只计一次；风险等级取实际最大后果，不是把症状的评分相加。
- 无 Finding 时可以为 approve（仅当关键 Concern 确实完成、规格缺口不影响判断、required 能力维度全部完成）。

## V2 Trace 最小字段

格式为合法 JSON。保留以下最小结构（附加自定义字段可以，但不应虚构）：

```json
{
  "schema_version": "2.0",
  "review_mode": "diff",
  "scope": "services/example/",
  "base_sha": "<real sha or N/A>",
  "head_sha": "<real sha or N/A>",
  "knowledge": {
    "status": "available|partial|unavailable|not_needed",
    "revision": "<sha|unversioned|N/A>",
    "references": []
  },
  "concerns": [
    {
      "id": "C01",
      "summary": "<actual behavioral concern>",
      "changed_locations": ["path:line"],
      "discover": "done|limited|not_applicable",
      "challenge": "done|limited|not_applicable",
      "coverage_critical": false,
      "limitations": [],
      "related_findings": []
    }
  ],
  "hypotheses": [{
    "id": "H-C01-01", "concern_ids": ["C01"],
    "invariant": "<actual invariant>", "outcome": "refuted",
    "feasibility": "refuted", "test_mode": "source_reasoning", "test_result": "not_run"
  }],
  "candidate_verdicts": [],
  "capability_coverage": {
    "required": ["behavior", "security"],
    "executed": ["behavior", "security"],
    "evidence": {"behavior": "C01 at path:line", "security": "C01 scope/relevance checked at path:line"}
  },
  "independent_verifier": false,
  "limitations": [],
  "report_findings": []
}
```

这只是最小契约示例，**不是实际检视输出**；运行时必须替换为真实数据。`capability_coverage` 的 `evidence` 应以每个能力名为键，列出对应的 Concern IDs、源码和简要结论（包含可证明的 N/A）。

`candidate_verdicts` 每条至少包括 `candidate_id`, `verdict`, `concern_ids`, `evidence`, `reason`, `finding_id`（无正式项则 null）；`report_findings` 为最终正式 Finding ID 数组。

## 验证顺序

1. 对照原始候选/Refute 记录，去重后统计 P0..P3 和每个 Finding ID。
2. 由 `conventions.md` 唯一公式计算 `score/risk_level/gate_decision`，不要由模型自行“觉得风险不大”选择 approve。
3. 检查 report `dimensions_*` 与 trace `capability_coverage` 相同，且有对应事实证据和独立限制说明。
4. 检查 trace 里 Concern 的 Discover/Challenge 状态与报告关于覆盖的措辞相符。
5. 运行 `scripts/validate_report.py`；它仅能校验**机器结构与可计算一致性**，不能替代真人确认风险事实。

## 不支持的情况

系统不具备运行环境时，标注 `not_run`，不能捏造测试输出；外部规格不可访问时不虚构条款；长期未决的接口合同冲突应单列开放问题，由人裁决。

每个 Concern 都须记录 `coverage_critical: true|false`：若该 Concern 的**实际检视限制**足以阻止安全合入判断，设为 `true`。Challenge 按风险自适应进行，**不规定每个 Concern 的反例数量**：

- `challenge: done` 表示确实调查过有价值的可反驳假设，因此 trace 中须有相应 Hypothesis 及检查结果；这不是要求强行生成一条“Bug”。
- `challenge: not_applicable` 表示已经判断没有实质攻防价值，必须提供 `challenge_justification`。不能仅因为未找到问题就标记不适用。
- `challenge: limited` 表示仍存在需要调查的关键假设，但受到权限、环境、上下文或预算限制；如该缺口影响合入判断，需设置 `coverage_critical: true`，门禁为 `insufficient`。

上述结构用于**核验实际做过什么**，而不是将调查转为固定配额或规定攻防轮数。
