# 代码检视报告 — {scope}（Round {round} / {review_mode}）

> Codecheck V2 自适应检视结果。机器头部为固定 17 字段（`schema_version: 2.0`）；`dimensions_*` 记录 V2 检视能力（`behavior` / `security` / `input` / `api`）的实际覆盖，不声明任何 Scanner 运行记录。行为上下文、外部知识引用、主动攻防及独立证伪详见同步交付的 `review_trace_*.json` 与 `refute_log_*.md`。

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "2.0"
  scope: "{scope}"
  round: {round}
  commit_id: "{commit_id}"
  change_id: "{change_id}"
  report_id: "{report_id}"
  date: "{date}"
  gate_decision: "{gate_decision}"
  risk_level: "{risk_level}"
  score: {score}
  dimensions_required: {dimensions_required}
  dimensions_executed: {dimensions_executed}
  findings_total: {findings_total}
  findings_by_severity: {P0: {p0}, P1: {p1}, P2: {p2}, P3: {p3}}
  gate_blockers: {gate_blockers}
  must_fix: {must_fix}
  followups: {followups}
```
<!-- codecheck-report-metadata:end -->

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **{gate_decision}** |
| 风险等级 | {risk_level} |
| 评分 | **{score}/100** |
| 阻塞项 | {gate_blockers_summary} |
| 必须修复（P0/P1） | {p0_p1_count} 项 |
| 建议跟进（P2/P3） | {p2_p3_count} 项 |

**一句话结论**：{executive_summary}

**检视范围**：{base_and_target}；**覆盖情况**：{concern_coverage}；**重要限制**：{material_limits_or_none}。

> 若 `insufficient` 且已证实存在 P0/P1，必须在此显著披露；不能用“无法评估”淡化已知阻塞项。

## 2. 扣分原因（仅 gate_decision=block 时保留）

> 共扣 **{total_deduction} 分**（封顶前），实际分数 = max(0, 100 - 扣分)。

| 扣分项 | 扣分数 | 问题 | 位置 |
|---|---|---|---|
{deduction_table_rows}

## 3. 必须立即处理（P0/P1）

| ID | 优先级 | 问题 | file:line | 触发路径 | 影响 |
|---|---|---|---|---|---|
{p0p1_table_rows}

> 无正式 P0/P1 Finding 时填写：**无。**

## 4. 建议本轮或下一轮处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
{followup_table_rows}

> 未经 Refute 确认的高价值候选应在本节表格**之后**独立标注为“未决风险”（不编号为正式 CC- Finding，不计入 `findings_total` 或 P3）。列明缺失的证据、人工需要裁决的问题和下一步验证建议。

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
{dimension_table_rows}

> 本表按检视能力维度（`behavior` / `security` / `input` / `api`）与真实 Concern 展开结果与关键说明；能力维度是覆盖声明，不是扫描任务。

## 6. 关键发现详情

> P0/P1 每条必须有完整卡片；P2/P3 建议全部列出以便审计。所有陈述必须从实际代码、规格或经授权测试中得到支持。

### [{finding_id}] {finding_title} ({severity})

- **位置**：`{file}:{line}`
- **关联 Concern**：{concern_ids}
- **触发路径**：{trigger_path}
- **影响**：{impact}
- **证据**：{evidence_with_real_locations}
- **规格/知识引用**：{verified_revision_path_section_or_not_applicable}
- **基线归因**：{introduced_or_exacerbated_or_existing_for_path_audit}
- **验证状态**：{static_reasoning_or_test_run_or_not_run}
- **建议**：{proportionate_remediation}
