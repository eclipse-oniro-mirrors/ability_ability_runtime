# 外部知识检索：按需引用，逐条核实

## 目的与外部来源

唯一职责：对当前代码检视出现的具体问题检索权威外部约束。Skill 不内置、复制或同步业务规格、编码规范、已知 Bug 列表。

知识库：`https://gitcode.com/abilitykit_public/abilitykit_sdd`。

- `standards/`：候选工程编码规范、约定、质量标准（是否属于强制规则要读正文确认）。
- `specifications/`：候选业务/架构/接口行为与生命周期规格；用以验证“系统应该如何表现”。
- 若仓库中有历史缺陷经验或解决方案目录，只能当作**搜索线索**，不能代替本次源码的触发证据，也不得默认其具有强制规范效力。

以上是目录角色约定，不假设实际文件名、编号、层级结构或每个目录都包含有关内容。

## 找到仓库

优先使用已有的可信本地检出，不私自下载：

1. `CODECHECK_KB_ROOT` 显式指定的本地绝对路径（最优先）。
2. 在工作区、仓库上级或用户给出的挂载目录中查找**明确名为 `abilitykit_sdd` 且含 `.git` 和目标目录**的已检出副本。
3. 若用户环境提供经授权的 GitCode 连接/文档检索能力，可通过该能力打开明确的文件。网页目录不可访问不代表规格不存在。
4. 以上都不可用：记录 `knowledge_status=unavailable`；当规格是判定正确性的必要输入时，标记受影响 Concern `limited`，门禁可能 `insufficient`；不可凭空补写规范。

示例（使用本地副本时）：

```bash
KB="${CODECHECK_KB_ROOT:?Set CODECHECK_KB_ROOT to the checked-out knowledge repository}"
git -C "$KB" rev-parse HEAD
rg --files "$KB/standards" "$KB/specifications" 2>/dev/null
```

需要初筛候选文件时，可使用随 Skill 提供的**只读本地检索工具**（仅返回候选，不自动采信匹配行）：

```bash
python3 skills/codecheck/scripts/kb_lookup.py --root "$KB" \
  --query "Ability 终止回调 生命周期" --category both --top 10
```

检索结果包含 `revision`, `path`, `line`, `section`, `snippet`。应继续打开对应文件及周边上下文核实适用范围；结果为空不等于没有相关规定。禁止把工具返回的片段等同于已阅读的完整规范。

不要默认远程 `main` 永远有效，**记录本次实际阅读的 SHA、相对路径和章节**；无法获取 SHA 时标记 `unversioned`，影响证据可信度。

## 什么时候触发检索

只在分析中出现需要外部事实才能裁决的**明确问题**时检索。例如：一个状态转换是否合法、错误返回是否为接口契约、跨设备行为是否有兼容约束、某个团队约定是否为强制规范。

检索流程是：

1. 用 **Concern + 实体/行为 + 待证问题** 形成具体查询（如“终止回调完成条件”），而不是简单以路径选择整份文档。
2. 先遍历仓库目录结构/元数据，缩小到相关文件；在候选文件里搜索术语、符号和行为，再展开上下文。
3. 阅读原文中的作用域、适用版本、前置条件、例外、`must/should` 级别，与目标代码的版本、流程逐一对照。
4. 检查是否有冲突或较新的适用规范；遇到冲突标记未决，不擅自选一个结果。
5. 仅把确认适用的原文规则映射为本次局部的 Review Contract（下例）；每条发现必须回链至原始来源。

```yaml
contract_id: "RC-C01-01"  # 本次 review 内部 ID，非知识库原生编号
concern_id: "C01"
question: "<待核对的问题>"
source_kind: "standard|specification|repository-rule|code-invariant"
repository: "abilitykit_public/abilitykit_sdd"
revision: "<verified-commit-sha or unversioned>"
path: "<真实相对路径>"
section: "<实际读过的章节/行>"
applicability: "<版本、调用方式、环境、例外>"
expected_behavior: "<忠实概括，不做无依据扩写>"
status: "applicable|not_applicable|conflicting|unverified"
```

**切勿制造规范编号**。内部 `RC-` 只是检视引用，和知识库文档编号不是一回事。

## 不同来源的优先级

用户明确的检视范围/项目仓库的权威指令 > 有明确适用版本和状态的正式业务/平台契约 > 已确认的本仓规范 > 代码周边事实/历史变更/经验案例。若权威来源相冲突，不根据便利程度选择“对模型更有利”的版本；在报告开放问题中显式暴露。

仅有经验案例或模型的常识性推断，不能支撑“违反了某条正式标准”的结论；但可以提出需要通过源码、类型/运行时保证或测试验证的候选问题。

## 缺失与失败模式

- `not_found`：已搜索相关范围未找到适用条款。不能推导“没有任何规范”。
- `unavailable`：环境未能访问仓库。不能伪称检索完成。
- `conflicting`：多个规范定义冲突。需记录双方实际引用与未决点。
- `not_applicable`：找到条款但确认版本/前置条件不符，记录理由。

当关键契约不可判定时，该 Concern 为 `limited`，相关 Finding 不得宣称已证实违规。其他能仅由代码逻辑证明的真实问题仍应继续检视并保留。
