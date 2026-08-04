---
name: api-scanner
description: |
  针对指定 Kit 的对外 API 做全量一致性审计、实现缺陷扫描与测试用例完备度评估，输出 Markdown + CSV 双格式。
  当用户提到"接口审计"、"API审计"、"资料一致性"、"实现bug扫描"、"一致性扫描"、"audit api"、
  "interface audit"、"扫描接口"、"深度扫描api"、"测试用例完备度"、"测试覆盖度评估"、"用例缺失"、
  "汇总到csv"、"导出csv"等，且上下文涉及对外 API（C API 或 JS API）时触发此 skill。
  即使用户只说"帮我扫一下 xxxKit"，也应触发。
  支持用户口头指定 Kit 名（如 abilityKit、arkuiKit），自动定位 docs/interface/framework/service/test 路径。
---

# API Scanner — 全量一致性审计、实现缺陷扫描与测试完备度评估

> **Pattern 引用声明**：本 scanner 加载 [`patterns/api-consistency.md`](../../patterns/api-consistency.md)（API-001~013 反模式）作为检查规则来源。三方一致性维度（A 签名/B 版本/C 错误码/D 参数校验/E 语义层 E1-E7/F 资料完整性）、服务侧审计要点（A 生命周期/B IPC/C 资源泄漏/D 错误码/E 副作用）、静态/动态混淆、BUILD.gn 同名模块区分等反模式均在 pattern 文件中定义。

对指定 Kit 的所有对外 API 执行**三轮**深度扫描：
- **第一轮（框架层）**：资料文档 × 接口定义 × 框架实现 三方一致性 + 参数校验完整性
- **第二轮（服务侧）**：生命周期/线程安全 + IPC 序列化 + 资源泄漏 + 错误码语义 + 文档/实现一致性
- **第三轮（测试用例）**：参数正常值 × 参数奇异值 × 边界值覆盖度评估，识别缺失场景与补充优先级

最终输出**两种格式**：
- **Markdown 报告**（`<kit-name>_api_audit.md`）：分章节叙事 + 三轮独立表格 + 关键缺陷汇总
- **CSV 宽表**（`<kit-name>_api_audit.csv`）：一个 API 一行，三轮结果横向并排

两种格式同步生成，数据等价，仅呈现方式不同。

> **Path B 声明**：本 scanner 直接被调用时，除产出下述 `<kit>_api_audit.md` + `<kit>_api_audit.csv` 双格式原始产出外，必须按 [`conventions.md`](../../conventions.md) §5–§13 规程生成符合 `codecheck_report_TEMPLATE.md` 的统一报告。审计标记 ❌/⚠️/✅ 按 conventions §7.2 归一化到 P0–P3：❌（含 E4 描述与实现不符、安全绕过）→ P0，❌（其他致调用失败、版本/错误码不一致）→ P1，⚠️（E5/E6/E7 不导致误用）→ P2，✅ 不报告。P0 项必须做 refute（见 conventions §13.2）。

---

## 路径速查表

> 以下路径基于 ability_runtime 总结，扫描其他 Kit 时按同构映射（`<subsystem>/<module>` 替换即可）。**框架侧"动态实现"与"静态实现"目录毗邻、文件名仅前缀不同，极易找错——扫错会导致整轮结论作废。**

| 类别 | 路径（ability_runtime 基准） | 说明 |
| -- | -- | -- |
| 资料文档（中文） | `docs/zh-cn/application-dev/reference/apis-ability-kit/` | `capi-*.md`（C API）+ `js-apis-*.md`（JS API） |
| JS API 接口定义 | `code/interface/sdk-js/api/` 及 `api/<子域>/` | `@ohos.app.ability.*.d.ts`（Stage 新）、`@ohos.application.*.d.ts`（FA 旧，多 deprecated）；`*.d.ets` / `*.static.d.ets` 为 static 侧，审 dynamic 时忽略 |
| C API 接口定义 | `code/interface/sdk_c/AbilityKit/` 下 `*.h` + `lib*.ndk.json` | `lib*.ndk.json` 含 `first_introduced` |
| **框架侧·动态 JS 实现（必扫）** | `code/foundation/ability/ability_runtime/frameworks/native/ability/native/ability_runtime/js_*.cpp` | NAPI 实现的 Stage 模型 Context/Ability 类。**dynamic 接口的真正框架实现入口** |
| **框架侧·动态 JS NAPI 模块（必扫）** | `code/foundation/ability/ability_runtime/frameworks/js/napi/<module>/` | NAPI 模块注册入口（`*_module.cpp`）+ 实现。Stage vs FA 用 `BUILD.gn` 的 `relative_install_dir` 区分（见 API-013） |
| 框架侧·C API 实现（必扫） | `code/foundation/ability/ability_runtime/frameworks/c/ability_runtime/` | C 接口包装层 |
| **框架侧·静态实现（跳过）** | `frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp` | ArkTS/Cangjie 静态语言实现，不纳入扫描。判定口诀见 [API-012](../../patterns/api-consistency.md) |
| **服务侧实现（必扫，R2 核心）** | `code/foundation/ability/ability_runtime/services/abilitymgr/src/`（含 `ability_manager_service.cpp`）+ `services/appmgr/src/`（含 `app_mgr_service.cpp`）+ `services/uripermmgr/src/` 等 | **R2 必须全量扫描，不能只扫客户端代理** |
| 测试用例（public） | `code/test/xts/acts/ability/<module>/` | JS：`*.test.ets`；C API：`entry/src/main/cpp/*.cpp` + `entry/src/ohosTest/ets/test/*.test.ets` |

> **AbilityKit 接口文件定位表**（d.ts × docs × 框架cpp 三方速查，103 模块 + 78 内部类型）：见 [`references/abilitykit_interface_files.md`](references/abilitykit_interface_files.md)。审计前先查此表定位三方路径，避免反复 Glob 探测找错文件。

---

## 触发条件

满足以下任一即触发：
- 用户直接提到"接口审计"、"API审计"、"资料一致性扫描"、"实现bug扫描"、"深度扫描api"
- 用户提到"扫描 xxxKit 的 api"或类似说法
- 用户问"这些接口实现有没有 bug / 资料对不对"
- 用户提供的输入是接口列表 / Kit 名称 / 现有审计报告需要补充

如果用户只问"这个文件/函数有没有问题"，不触发本 skill（属于普通代码审查）。

## 输入识别

用户输入通常包含以下信息（缺失时主动询问或自动探测）：
1. **Kit 名**：如 abilityKit、arkuiKit、nativeKit（用于定位 docs/interface 目录）
2. **审计范围**：默认全量；若用户指定子集（如"只扫 capi"），按指定子集
3. **审计深度**：默认三轮全跑；除非用户明确说"只看资料一致性"
4. **输出路径**：默认写到 `<cwd>/<kit-name>_api_audit.md`

---

## 工作流程（必须按顺序执行）

### Phase 1 - 准备阶段

#### 1.1 定位 Kit 范围

> 具体路径见上方"路径速查表"。下方为通用规则。

- C API 接口定义：`code/interface/sdk_c/<KitNamePascal>/` 下所有 `*.h`
- JS API 接口定义：`code/interface/sdk-js/api/.d.ts` 中对应的 `@kit.<KitName>` 模块
- **资料文档（必须定位并实读，禁止仅凭 d.ts 推导资料描述）**：`docs/zh-cn/application-dev/reference/apis-<kit-name>/` 下 `capi-*.md` 和 `js-apis-*.md`。**docs 文件名与 d.ts/.h 名称不一一对应**（如 `UIAbilityContext.d.ts` → `js-apis-inner-application-uiAbilityContext.md`），**必须用 Glob 按类名/方法名/namespace 搜索定位**，不能靠猜文件名。定位后必须 Read 实读。
- NDK 版本声明：`code/interface/sdk_c/<KitNamePascal>/` 下所有 `lib*.ndk.json`（含 first_introduced）
- **框架实现（动态 JS，必扫）**：`frameworks/native/<module>/.../js_*.cpp` + `frameworks/js/napi/<module>/` + `frameworks/c/<module>/`
- **框架实现（静态，跳过）**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/` 及 `ets_*.cpp`/`cj_*.cpp`（判定规则见 [API-012](../../patterns/api-consistency.md)）
- **服务实现（R2 必扫）**：`code/foundation/<subsystem>/<module>/services/<mgr>/src/` 下所有 `*.cpp`
- 测试用例：`code/test/xts/acts/<subsystem>/<module>/`

若用户未指定 Kit，从打开的文件路径或上下文推测；推测失败时询问。

#### 1.2 列出全量 API 清单
- 对每个 `.h` 提取所有对外函数声明、typedef、枚举
- 对每个 `.d.ts` 提取所有 export 的接口、方法、属性
- 与 NDK JSON 对照，得到每个 API 的起始版本、是否 system api

#### 1.2.1 按接口语言类型过滤（强制）

JS API 按 `static` / `dynamic` / `dynamic&static` 标注过滤：
- **static only**：**直接跳过**，不进入扫描清单（对应 ArkTS/TS 静态语言接口）
- **dynamic**：**必须扫描**（对应 JS 动态语言接口）
- **dynamic & static**：**必须扫描**，聚焦 dynamic 部分
- **未标注**：默认按 dynamic 纳入
- C API（`.h`）默认全部纳入，不受本规则约束

> 框架侧实现文件同样按此过滤，判定口诀见 [API-012](../../patterns/api-consistency.md)：`js_` 前缀 + NAPI 签名 = 动态（扫）；`ets_`/`cj_` 前缀 = 静态（跳过）。

被跳过的 static-only 接口需在最终报告"审计范围"章节注明数量与名单。

#### 1.3 创建任务清单
按子模块创建扫描任务 + "服务侧深度审计" + "测试用例完备度审计" + "汇总报告"任务。

### Phase 2 - 第一轮：框架层三方一致性扫描

> **三方 = 资料文档（docs `js-apis-*.md`/`capi-*.md`）× 接口定义（`d.ts`/`.h`）× 框架实现（`js_*.cpp`/`frameworks/c/`）。** 本轮逐 API 按 [`patterns/api-consistency.md`](../../patterns/api-consistency.md) API-001~006 核对三方一致性，并扫框架实现内部基础 bug。"资料"特指 docs 仓的 md 文件，**不是 d.ts 注释**——必须 Glob 定位 docs md 并 Read 实读，禁止仅凭 d.ts 推导"资料描述"列。

#### 2.1 三方一致性核对（按维度，每个 API 必查）

按 [API-001](../../patterns/api-consistency.md)（签名一致性）、[API-002](../../patterns/api-consistency.md)（起始版本一致性）、[API-003](../../patterns/api-consistency.md)（错误码一致性）、[API-004](../../patterns/api-consistency.md)（参数校验一致性）、[API-005](../../patterns/api-consistency.md)（E1-E7 语义层）、[API-006](../../patterns/api-consistency.md)（资料存在性与完整性）逐项核对。

**输出要求**：
- **"资料描述"列必须引用 docs md 行号 + 摘录原文关键短语**。格式：`docs <md文件名>:<行号> "<原文片段>" + 一句话概括`。**禁止凭方法名概括**。
- **A/B/C/D/E/F 六项必须逐 API 全部扫过，不得抽样**：即使结论是"一致"也必须显式记录对照证据（如 `✅ 签名一致：docs md:NNNN 参数表 = d.ts:NN 参数声明 = cpp:NN 解析`）。
- **E 项（语义层 E1-E7）不得跳过**：若某 API 的 docs 描述过于简略导致 E1-E7 全部不适用，需显式注明 `E1-E7 不适用：docs 仅声明签名无功能描述`。
- 优先级：**E 项（语义不一致）> A/B/C/D 项 > F 项**。E4（描述与实现不符）一律 ❌。
- 每条发现必须给出：docs md file:line + 代码 file:line + 一句话差异说明。
- **docs md 的 markdown 格式问题（`<sup>` 闭合、`<br/>` 等）不在审计范围内**。

#### 2.2 框架实现内部 bug（基础）
内存分配/释放配对、整数溢出（strlen 赋值给 int32_t 等）、浅拷贝 vs 深拷贝（特别是 char*）、拼写错误。

#### 2.3 docs × 代码比对覆盖率强制自检（必做）

报告生成前必须完成以下 4 条自检，任一未通过必须补扫：

1. **资料描述列行号引用率 = 100%**：R1 表格每个 API 的"资料描述"列必须含 `docs <md文件名>:<行号>` 形式引用。
2. **docs 章节覆盖率 ≥ 95%**：每个 API 都在对应 docs md 章节做过实读核对。
3. **E1-E7 语义层核对显式记录**：至少有一条 E 项发现，或显式声明全部不适用。
4. **三方比对证据显式化**：✅ 一致的 API 必须附 d.ts/docs/cpp 行号对照证据。

自检结果在报告"第一轮"章节末尾声明：
```
> **R1 docs 比对自检**：资料描述列行号引用 N/N（100%）；docs 章节覆盖 N/N（100%）；E1-E7 语义层核对完成（发现 E3 × n、E4 × n）；三方比对证据已显式化。
```

未达标时**禁止生成 CSV**。

### Phase 3 - 第二轮：服务侧深度扫描

**这是本 scanner 的核心价值。** 第一轮只覆盖框架层基础参数校验，必须深入到服务侧才能发现真正的 bug。

#### 3.1 追踪调用路径

对每个 C API：`frameworks/c/<module>/src/<file>.cpp` → C++ 包装类（AbilityManagerClient 等）→ IPC → `services/<module>/src/<file>.cpp` → 实际逻辑

对每个 dynamic JS API：`frameworks/native/<module>/.../js_<class>.cpp`（`JsXxx::Method` 入口）→ `OnXxx` 处理函数 → `AbilityManagerClient::GetInstance()->Xxx(...)` → IPC → `services/abilitymgr/src/ability_manager_service.cpp` stub → 实际逻辑

> **服务侧目录必须全量扫描，不能只扫客户端代理**。必须覆盖 `services/abilitymgr/src/*.cpp`（所有文件，尤其 `ability_manager_service.cpp`、`*_stub.cpp`、`*_manager.cpp`）+ `services/appmgr/src/*.cpp`。曾发生只扫客户端代理而漏掉 `ability_manager_service.cpp`，导致 R2 服务侧 bug 全部漏报。

#### 3.2 服务侧审计要点

按 [API-008](../../patterns/api-consistency.md)（生命周期/线程安全）、[API-009](../../patterns/api-consistency.md)（资源泄漏）、[API-010](../../patterns/api-consistency.md)（IPC 序列化）、[API-011](../../patterns/api-consistency.md)（API 行为/副作用）、[API-007](../../patterns/api-consistency.md)（框架实现内部 bug）逐项扫描。详细检查项见 pattern 文件。

#### 3.3 验证发现的 bug
对每个服务侧发现的 bug，**必须**用 Read/Grep 读取原始代码二次确认：函数定义文件:行号、触发条件、影响范围、验证结论（✅ 真实 / ❌ 误报 / ⚠️ 部分正确）。未经验证的 bug 不要写入最终报告。

**R2 强制自检**：报告生成前 Grep 确认引用的 file:line 中至少包含一处 `services/` 路径下的文件；若全部在 `frameworks/` 下，说明服务侧未扫到，必须补扫。

### Phase 4 - 第三轮：测试用例完备度审计

#### 4.1 定位测试代码
- **public API 测试**：`code/test/xts/acts/<subsystem>/<module>/` 下 `.test.ets`（JS）/ `entry/src/main/cpp/*.cpp` + `entry/src/ohosTest/ets/test/*.test.ets`（C API）
- **system API 测试**：通常缺失，需在报告中明确标注"system api 测试未提供"

#### 4.2 测试覆盖维度（每个 API 必查）
- **A. 参数正常值覆盖**：典型用法、返回值/输出参数校验、多次调用累积效果
- **B. 参数奇异值覆盖**：每个指针参数单独传 nullptr、数值边界值（INT32_MIN/MAX/0/-1、NaN/Infinity、SIZE_MAX）、字符串（nullptr/空/超长/特殊字符）、buffer（bufferSize=0/极小/恰好/极大）
- **C. 生命周期与并发覆盖**：销毁后再访问（UAF）、重复创建/销毁（double-free）、重复注册/解注册回调、并发调用、回调中再调用 API、内存泄漏（循环创建不销毁）
- **D. 错误路径覆盖**：失败路径资源释放、异步回调超时/失败、IPC 失败客户端行为、权限拒绝路径

#### 4.3 评估完备度评级
- **A（≥80%）**：正常值、奇异值、边界值、错误路径均覆盖
- **B（50-80%）**：正常值覆盖完整，奇异值部分覆盖
- **C（<50%）**：仅测 nullptr 或仅测正常值
- **D（几乎无覆盖）**：零测试或仅零星几个 nullptr 用例

#### 4.4 识别缺失场景
- **P0（必须补）**：零覆盖的核心功能路径；成功路径完全未测；关键失败路径未测；与文档矛盾的测试
- **P1（建议补）**：边界值、特殊字符、非法枚举值、关键并发场景
- **P2（可选）**：极少触发的边界、防御性测试

#### 4.5 关键检查清单
- ✅ 是否有"正常 context + 验证返回内容"的用例（不只是 nullptr 三件套）
- ✅ 销毁后访问是否被测试
- ✅ 异步 API 的成功回调是否被测试（不只是错误码）
- ✅ 测试是否与文档/头文件声明一致
- ✅ 是否存在测试用例调用了错误函数
- ✅ system api 是否标注"测试未提供"

### Phase 5 - 并行加速

当 API 数量超过 30 个时，按子模块拆分用 Agent 并行扫描（每个 Agent 负责一组 API 的三轮扫描），主 Agent 负责汇总和二次验证。

---

## 输出格式

### Phase 6 - Markdown 报告

默认写到 `<cwd>/<kit-name>_api_audit.md`。若已存在，追加新章节而非覆盖。

报告结构（严格按此生成）：

```markdown
# {KitName} API 一致性与实现缺陷审计报告
- **审计范围** / **审计维度** / **起始版本来源** / **系统 API 判定** / **生成日期**

> 扫描结果三档：✅ 一致（三方匹配）/ ⚠️ 轻微问题（文档/拼写/可读性）/ ❌ 具体 bug（含 file:line）

## 第一轮：框架层三方一致性扫描
### 一、<文件名>.h
| 文件 | 接口 | 类型 | 系统API | 起始版本 | 资料描述 | 扫描结果 | 修复方案 |

## 第二轮：服务侧实现深度审计
### 一、<模块名> 服务侧
| 文件 | 接口 | 类型 | 系统API | 起始版本 | 资料描述 | 深度扫描结果 | 修复方案 |

## 第三轮：测试用例完备度审计
### 一、<模块名> 测试覆盖
| 文件 | 接口（可分组） | 参数正常值覆盖 | 参数奇异值覆盖 | 缺失场景（优先级） | 完备度评级 | 推荐补充用例 |

## 关键缺陷汇总（按风险等级）
### 高危 / 中危 / 低危 / 资料一致性

## 测试覆盖关键缺失汇总（按补充优先级）
### 高优先级(P0) / 中优先级(P1) / 低优先级(P2)

## 修复与补充优先级建议
```

**R1/R2 表格 8 列**：文件 | 接口 | 类型(JSAPI/CAPI) | 系统API(Y/N) | 起始版本 | 资料描述(docs md 行号引用+原文片段+概括) | 扫描结果(✅/⚠️/❌ 含 file:line) | 修复方案

**R3 表格 7 列**：文件 | 接口(可分组) | 参数正常值覆盖(✅/⚠️/❌) | 参数奇异值覆盖(✅/⚠️/❌) | 缺失场景(含 P0/P1/P2) | 完备度评级(A/B/C/D) | 推荐补充用例

**扫描结果列自包含要求（最高优先级）**：单元格内容必须独立可读——读者只看这一格就能理解问题位置、触发条件、影响、修复方向。**禁止**只写编号引用（如 `❌ H1+H2`），必须复述完整 file:line + 问题描述 + 影响。编号可作前缀索引但必须紧接完整描述。

**编号约定**：R1 高危 H1/H2…、中危 M1/M2…、低危 L1/L2…；R2 服务侧加 `S-` 前缀；R3 测试缺失用 `T-P0-N`/`T-P1-N`/`T-P2-N`。编号仅用于汇总章节排序检索，**不能代替描述**。

### Phase 7 - CSV 宽表

默认写到 `<cwd>/<kit-name>_api_audit.csv`。UTF-8，逗号分隔，**所有字段用双引号包裹**。

**15 列结构**：
```
文件,接口,类型,系统API,起始版本,资料描述,R1_扫描结果,R1_修复方案,R2_深度扫描结果,R2_修复方案,R3_正常值覆盖,R3_奇异值覆盖,R3_缺失场景,R3_评级,R3_推荐补充用例
```

**数据合并规则**：以 R1 为 master 列表（CSV 行数 = R1 API 总数）。R2/R3 按 API 名精确匹配或分组匹配合并；未提及的 API，R2 列填 `✅ 一致`/`无需修复`，R3 列填 `-`。

**字段清洗**：去除 Markdown 标记（`**`/`` ` ``/`<br>`）；字段内 `"` 转为 `""`；`\n` 转空格；file:line 证据完整保留；**编号引用必须完整展开**（与 §6.3 自包含要求一致，禁止只搬编号或"同前/同上"）。

**抽样验证（必做）**：CSV 生成后抽样 5 个 API 验证合并正确性（三轮都一致的、R1+R2 都有 bug 的、R3 评级 D 的、R2/R3 分组行的各 1 个）。

**MD 是权威源，CSV 是 MD 的派生视图**。

---

## 执行约束

1. **不要只扫框架层**：R1"一致"不代表实现无 bug，必须深入服务侧
2. **不要写无 file:line 的结论**：每个 bug 必须有具体定位
3. **不要跳过二次验证**：服务侧 bug 容易误报，必须 Read 原始代码确认
4. **不要混淆轮次**：每轮发现独立编号，不跨轮重复
5. **不要把"测试调用了 API"等同于"覆盖了功能"**：必须验证测试是否校验了返回值、输出参数、副作用
6. **测试审计要对照文档/头文件**：测试期望的错误码与文档/头文件声明一致才算覆盖，矛盾的测试本身是 P0 缺陷
7. **system API 测试缺失要明确标注**：XTS 通常不提供 system api 测试，需显式说明

## 特殊情况

- **用户只指定一个具体 API**：仅扫描该 API，三轮全跑，MD + CSV 都生成
- **用户要求补充现有报告**：读现有报告识别已扫描范围，只追加未覆盖部分；CSV 需整体重新生成
- **用户只要 CSV**：仍需先完成三轮扫描，CSV 是 MD 的派生
- **找不到服务实现目录**：标注"未找到服务实现，仅完成框架层扫描"，不要编造
- **找不到测试用例目录**：标注"未找到 XTS 测试套件，测试覆盖审计跳过"，不要编造
- **API 数量超过 100 个**：按子模块并行扫描，CSV 合并由主 Agent 统一处理
- **跨 Kit 引用**：如 abilityKit 引用了 ability_base 的 Want，应一并审计被引用侧
