# CHANGELOG — codecheck skill

格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/)；所有条目来自 git 提交历史（附短 SHA 可回溯），不虚构版本号。版本标记沿用真实存在的报告契约标记：**schema 1.0（V1 时代）→ V2 / schema 2.0（现行）**。

---

## [V2 / schema 2.0] — 2026-11-07（`!20933` 合入 master，`26d9cd4393`）

V2 重构在 `codeskill` 分支分三步完成，同日合入：

| 步骤 | Commit | 内容 |
|---|---|---|
| 1 | `f7431308b1` | 升级为 V2 自适应检视技能：新增 `references/` 五协议（review-workflow / knowledge-retrieval / adversarial-review / refutation-protocol / report-contract）、`scripts/kb_lookup.py`（外部知识库只读检索）、`scripts/validate_report.py`（报告契约校验）；删除 V1 的 `generate_report.py`、`review_policy.py`、`export_text.py` 及其测试（+1355/−1470） |
| 2 | `c7e06c47c9` | 报告契约升级为 **schema 2.0**，移除 v1 兼容垫片：`conventions.md`、报告模板、README、SKILL 同步改写 |
| 3 | `0f4d39c6eb` | 确立 V2 终形态：删除全部 `scanners/`（input-scanner / logic-scanner / security-scanner）及 deep-scan 遗留脚本（累计 −4816 行） |

### 重构要点（V1 → V2）

- **任务组织**：固定 Scanner + 路径路由 → 行为 Concern / 跨 Concern 交互；不再维护内置 Patterns / G 类规则。
- **知识来源**：内置规则 → 外部 [`abilitykit_sdd`](https://gitcode.com/abilitykit_public/abilitykit_sdd) 的 `standards/`、`specifications/` 按需检索（`kb_lookup.py` 支持本地副本 `CODECHECK_KB_ROOT`）。
- **发现方式**：按规则类型探索 → 从不变量、代码行为与上下文生成可证伪假设；攻防不设固定反例数量或轮数。
- **报告契约**：17 字段机器元数据沿用字段名与评分权重（30/12/5/2），但 `schema_version` 升为 2.0，`dimensions_*` 取值域改为 V2 能力名 `behavior/security/input/api`；决策矩阵改为**覆盖不足优先 `insufficient`（即使已有 P0）+ 累计扣分 <70 直接 `block`**。
- **误报控制**：按严重性抽样复核 → 所有正式候选强制证伪核验（触发、影响、归因、去重）。
- **新增 `evals/`**：真实历史缺陷回放与消融测试规程（`test_gate_contract.py`、`test_kb_lookup.py`）。

### 破坏性变更

- schema 2.0 报告与 v1 **不互认**：旧版 schema 1.0 报告只作存档对照，不再作为门禁输入；依赖旧字段语义的消费端须按新取值域与决策矩阵自行适配。
- 机器报告不声明任何 Scanner 运行记录；`dimensions_*` 是覆盖声明，不是扫描任务调用记录。

---

## [V1 终形态：九阶段统一检视] — 2026-10-30（`c458c248e9`）

- 统一九阶段检视流水线（`orchestrator/SKILL.md`）：① 固定 PR head/base 与历史 → ② 获取意图与既有契约 → ③ 建立影响链与所需维度 → ④ 按需查知识并统一检视 → ⑤ 针对假设二次扩搜 → ⑥ 同类合并与定向测试 → ⑦ 集中双向 Refute → ⑧ 历史过滤后定级与门禁 → ⑨ 原格式报告与交付历史。
- 完善证据、去重与门禁规则；目录结构为 `orchestrator/`（SKILL + refute-rules）+ `patterns/INDEX.md` + `scripts/{export_text,generate_report,review_policy}.py` + `tests/`。

---

## [报告契约统一期] — 2026-08（`4d1d22493d`…`5e294f498c`）

- 08-01 `4d1d22493d`：优化代码检视报告模板。
- 08-03 `b244f140d6`：重构 codecheck 三层架构，统一报告格式到门禁模板。
- 08-06 `d9c4117c1d`：报告模板精简为 6 节结构，评分规则收口至 `conventions.md`。
- 08-07 `7328b24b38`：修复模板重构遗留的 3 项 P1 一致性问题。
- 08-31 `5e294f498c`：新增 G16（BundleEventCallback 预置应用 OTA 盲区）缺陷规则。

---

## [多 Skill 并存期] — 2026-07（`11ba23efc8` 可见结构）

- `skills/codecheck/` 下为多个独立 Skill 并存：`codecheck-orchestrator`、`deep-scan`、`external-input-audit`、`high-impact-bug-audit`、`logic_analyzer`、`security_review`、`api-audit`。
- `security_review` 规则积累：
  - 07-04 `11ba23efc8`：杂项修复；
  - 07-23 `c9723f2351`：IPC 鉴权拆分为独立检视方向并引入 references；
  - 07-25 `3c086413f1`：新增 G11–G15 缺陷模式、补充热点模块；
  - 07-30 `01f64cc9b5` / `2727e7de97`：扁平化为 reference 风格并重命名目录；报告模板优化。

---

## 维护约定

- 新条目置顶，标注日期与 commit 短 SHA；行为契约变更须同时在 `conventions.md` / `report-contract.md` 落地，二者不一致时以 `conventions.md` 为准。
- 涉及报告 schema 取值域、评分公式或决策矩阵的变更属**破坏性变更**，须在条目中显式声明并评估存量报告消费端的适配。
