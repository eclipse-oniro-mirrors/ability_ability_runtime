---
name: external-input-audit
description: >
  对指定代码路径执行"外部输入 → 持久化"全链路健壮性审计。
  识别从外部数据源（IPC/Parcel、HTTP 请求体、CLI 参数、配置文件、网络 socket）
  到持久化目标（数据库、文件、缓存、日志）的危险链路，评估写入侧防御、
  读取侧恢复、并发安全与日志注入等全生命周期风险。
  触发方式：
  1. 用户输入 /external_input_audit {path}
  2. 用户用自然语言表达"外部输入排查"、"输入接口审计"、"持久化安全检查"
     "数据入口审计"、"输入校验扫描"、"接口健壮性排查"等意图。
  输出：按 P0/P1/P2 优先级排序的 Excel 风险清单。
---

# 外部输入接口健壮性审计 (External Input Audit)

## 概述

以 **持久化锚点**（sink）为起点，**反向追踪**数据来源，只标记"可达外部输入"的链路。
直接过滤内部数据持久化的噪音，聚焦真正危险的"外部数据写入不可逆存储"路径。

## 核心原则

- **方向**：从 sink（持久化点）反向追踪到 source（外部输入），不扫描纯内部链路
- **优先级**：P0（DB/文件持久化）> P1（缓存/MQ）> P2（日志）
- **证据要求**：每个发现必须能写出"给定输入 X，通过入口 Y，经路径 Z，写入目标 W"
- **输出**：Excel 风险清单，含写入侧 + 读取侧双维度评估

## 工作流

### Step 1: 持久化锚点定位 (Sink Discovery)

扫描目标路径内所有持久化操作，建立 sink 清单。

#### 搜索命令

```bash
# 数据库操作（SQLite / RDB Store）
rg -n "Insert\s*\(|Update\s*\(|Delete\s*\(|ExecuteSql\s*\(|execSQL" --type cpp

# 文件写入
rg -n "fopen|fwrite|fputs|fprintf|open\s*\(|write\s*\(|ofstream|SaveToFile|WriteToFile" --type cpp

# 键值/偏好设置持久化
rg -n "SetValue|SetString|SetInt|PutString|PutInt|Save\s*\(|Flush\s*\(|Commit\s*\(" --type cpp

# 序列化落盘
rg -n "Marshalling|Parcel::Write|Serialize|ToJson|dump\s*\(|WriteParcelable" --type cpp

# 日志写入
rg -n "TAG_LOG|HILOG_INFO|HILOG_WARN|HILOG_ERROR|HILOG_DEBUG|printf\s*\(" --type cpp
```

#### 输出 Sink 清单格式

```text
SINK-001 | src/user_mgr.cpp:342 | InsertUserInfo(user_name, email, avatar_path)
SINK-002 | src/config.cpp:128  | fopen(profile_path, "w") + fprintf
SINK-003 | src/logger.cpp:56   | HILOG_INFO("user_%{public}s_action", input)
```

记录每个 sink 的：文件路径、行号、写入操作类型、被写入的变量名列表。

### Step 2: 反向污点追踪 (Reverse Taint Tracking)

对每个 sink 变量做反向数据流分析，判断是否可达外部输入源。

#### 外部输入源识别

对该代码库，重点关注以下 source 类型：

| Source 类型 | 代码特征 | C++ 搜索模式 |
|------------|---------|-------------|
| **IPC/Parcel 反序列化** | Parcel::ReadString/ReadInt32/ReadParcelable | `Parcel::Read\w+\(` |
| **NAPI 接口参数** | napi_get_value_*, NaPiGetValue* | `NaPiGetValue|GetParam|napi_get_value` |
| **HTTP/网络请求** | HttpRequest/recv/read | `HttpRequest|recv\s*\(|recvfrom\s*\(` |
| **CLI 参数** | aa_start/AATools/argc/argv/getopt | `argc|argv|getopt|GetOption` |
| **配置文件读取** | ReadFileToJson/ReadFileString/LoadConfig | `ReadFile|LoadConfig|FromJson\(|ReadJsonFile` |
| **系统属性** | GetParameter/GetSystemProperty | `GetStringParameter|GetIntParameter` |
| **环境变量** | getenv | `getenv\s*\(` |
| **Bundle/包信息** | GetAppInfo/GetBundleInfo（不可信来源） | `GetBundleInfo|GetAppInfo` |
| **DataShare** | DataShare query/insert/update 回调 | `DataShareResultSet|DataShareValuesBucket` |

#### 追踪步骤

1. 对每个 sink 变量，向上游追溯，记录所有赋值和传递路径
2. 在路径上寻找校验函数（长度检查、类型校验、白名单过滤、编码转义、参数化查询）
3. 标记路径上的防护代码：
   - `PreparedStatement` / ORM 调用（SQL 注入防护）
   - `CanonicalizePath` / `normalize` 调用（路径遍历防护）
   - `SetMaxValue` / 长度边界检查
   - 正则/类型校验

#### 判定规则

| 判定 | 条件 |
|------|------|
| **High Risk (可达)** | source → ... → sink，路径中无任何校验 |
| **Medium Risk (可达但部分校验)** | source → ... → 校验不完整 → sink |
| **Low Risk (可达但有充分校验)** | source → ... → 多层校验 → sink |
| **No Risk (不可达)** | sink 变量来源为内部常量/计算结果，无法追溯到外部输入 |

### Step 3: 接口画像 (Interface Profiling)

对 High/Medium Risk 的链路，提取结构化信息。不要求填写 YAML 模板，而是逐一确认以下维度：

#### 3A. 写入路径 (Write Path)

逐项确认：
- [ ] **持久化目标**：具体库名/表名/文件路径/缓存 key 前缀
- [ ] **写入方式**：参数化查询？字符串拼接？ORM？
- [ ] **事务保护**：有事务包裹？批量写入部分失败处理策略？
- [ ] **入参校验**：长度限制？类型检查？白名单？正则匹配？
- [ ] **并发安全**：乐观锁/悲观锁？幂等键？重入保护？
- [ ] **目标安全**：写入路径是否与配置文件声明一致？权限是否合理？是否存在越界/覆盖系统路径的风险？

#### 3B. 读取路径 (Read Path) — 独立维度

逐项确认：
- [ ] **完整性校验**：读取时是否有 checksum/schema_version 校验？
- [ ] **输出编码**：从 DB 读出的数据直接渲染前是否做了编码/转义？
- [ ] **反序列化安全**：使用的反序列化方式是否安全？(pickle/Java原生序列化 = 高危)
- [ ] **损坏数据处理**：遇到损坏/异常数据的处理策略是什么？
- [ ] **版本兼容**：旧版代码读取新版数据结构是否安全？

#### 3C. 异常行为与恢复

逐项确认：
- [ ] **异常数据隔离**：是否有脏数据隔离区（quarantine table / .bad file / DLQ）？
- [ ] **写入原子性**：多表/多文件写入是否保证原子性？
- [ ] **降级策略**：持久化层不可用时如何处理？
- [ ] **幂等性保护**：重复提交是否导致重复持久化？
- [ ] **审计与溯源**：是否保留原始输入快照用于事后溯源？
- [ ] **恢复手段**：数据被异常篡改后的恢复手段和恢复窗口？

### Step 4: 静态漏洞分析 (Static Vulnerability Analysis)

对每条 High Risk 链路，检查以下攻击模式是否可能被利用：

#### C++ 高危检查项

| 攻击类型 | 检查内容 | 风险信号 |
|---------|---------|---------|
| **SQL 注入** | 是否存在 `sprintf(sql, ..., input)` 或字符串拼接 SQL？ | 无 PreparedStatement |
| **路径遍历** | 外部输入是否构成文件路径的一部分？是否调用了 `CanonicalizePath`？ | 路径含 `../` 未过滤 |
| **命令注入** | 外部输入是否传递给 `system()`/`popen()`/`exec*()`？ | 直接拼接命令字符串 |
| **格式字符串** | 外部输入是否作为 `printf/sprintf/fprintf` 的 format 参数？ | format 参数非字面量 |
| **日志注入 (CRLF)** | 外部输入直接写入日志，未过滤 `\r` `\n`？ | 日志解析器可能被欺骗 |
| **Null Byte 注入** | `std::string` 含 `\0` 但按 `c_str()` 边界截断？ | C/C++ 字符串终止符利用 |
| **整数溢出** | size/offset/index 来自外部输入且无边界检查？ | 可导致越界读写 |
| **反序列化炸弹** | JSON/二进制数据无深度/大小限制直接解析？ | 可导致 OOM/栈溢出 |
| **TOCTOU** | 先检查文件存在再打开？先 SELECT 再 UPDATE？ | 时间窗竞态 |
| **Use-After-Free** | 外部输入触发的异步回调持有了已释放的原始指针？ | 生命周期不匹配 |

### Step 5: 恢复机制审查 (Resilience Checklist)

对每条 High/Medium Risk 链路，确认以下 10 项：

1. **异常数据隔离**：是否将异常输入写入隔离区而非直接丢弃？
2. **写入原子性**：多目标写入是否保证 all-or-nothing？
3. **批量部分失败**：批量写入第 K 条失败后前 K-1 条是否回滚？
4. **数据版本兼容**：Schema 带版本号，读写时做兼容性分支？
5. **读取侧校验**：读出数据时是否有完整性/合法性检查？
6. **反序列化安全**：使用 JSON/ProtoBuf 等安全格式，而非 pickle/Java 原生序列化？
7. **幂等性保护**：存在去重键或幂等键防重复写入？
8. **降级策略**：持久化层不可用时有明确的拒绝/排队策略？
9. **审计溯源**：异常数据保留原始输入哈希/摘要？
10. **恢复手段与窗口**：有明确的数据恢复 SOP 和时间窗口？

## 风险等级判定

| 等级 | 条件 |
|------|------|
| **P0 (Critical)** | 外部输入 → DB/文件写入，无校验，无事务，无恢复机制，存在注入攻击面 |
| **P1 (High)** | 外部输入 → DB/文件写入，校验不完整，或存在路径遍历/反序列化风险 |
| **P2 (Medium)** | 外部输入 → 缓存/日志写入，校验不完整，或并发安全缺失 |
| **P3 (Low)** | 可达外部输入但有多层防护，或仅为只读操作 |

## 输出格式

### Step 6: 生成 Excel 报告

使用 `.xlsx` 文件，工作簿名称 `外部输入风险清单`，必须包含以下列：

| 列名 | 内容要求 |
|------|---------|
| `文件路径` | 问题所在源文件的相对路径 |
| `行号` | 问题代码位置，支持 `123` 或 `120-130` |
| `链路ID` | 唯一标识，如 `CHAIN-001` |
| `问题概述` | 简短标题（一句话） |
| `外部输入源` | 输入来源（IPC/Parcel / HTTP / CLI / 配置文件 / socket） |
| `持久化目标` | 写入目标（DB表 / 文件路径 / 缓存key / 日志） |
| `校验状态` | ❌无校验 / ⚠️部分校验 / ✅充分校验 |
| `攻击面` | 可能的攻击类型（SQL注入/路径遍历/日志注入/反序列化/并发） |
| `写入侧评估` | 写入路径防护情况 |
| `读取侧评估` | 读取路径防护情况 |
| `恢复机制` | 恢复能力评估 |
| `风险等级` | P0 / P1 / P2 / P3 |
| `修复建议` | 具体、可操作的修复方案 |

### 样式规范

- 首行表头：深蓝底（`4472C4`）白字，居中加粗
- 所有单元格：细边框，自动换行，垂直顶对齐
- `风险等级` 列按等级着色：P0=红色（`FF0000`）、P1=浅红（`FF7F7F`）、P2=橙色（`FFC000`）、P3=黄色（`FFFF00`）
- 冻结首行
- 列宽参考：文件路径 50、行号 12、链路ID 14、问题概述 40、外部输入源 18、持久化目标 30、校验状态 12、攻击面 25、写入侧评估 40、读取侧评估 40、恢复机制 40、风险等级 10、修复建议 50

### 生成脚本

```python
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

risk_colors = {
    "P0": "00FF0000", "P1": "00FF7F7F", "P2": "00FFC000", "P3": "00FFFF00"
}

wb = openpyxl.Workbook()
ws = wb.active
ws.title = "外部输入风险清单"

# Styles
header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="004472C4", end_color="004472C4", fill_type="solid")
data_font = Font(name="Calibri", size=11)
wrap_top = Alignment(wrap_text=True, vertical="top")
header_align = Alignment(wrap_text=True, vertical="center", horizontal="center")
thin_border = Border(
    left=Side(style="thin"), right=Side(style="thin"),
    top=Side(style="thin"), bottom=Side(style="thin"),
)

headers = ["文件路径", "行号", "链路ID", "问题概述", "外部输入源", "持久化目标",
           "校验状态", "攻击面", "写入侧评估", "读取侧评估", "恢复机制", "风险等级", "修复建议"]
widths = [50, 12, 14, 40, 18, 30, 12, 25, 40, 40, 40, 10, 50]

for col_idx, (header, width) in enumerate(zip(headers, widths), 1):
    ws.column_dimensions[get_column_letter(col_idx)].width = width
    cell = ws.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align
    cell.border = thin_border

# Write findings (example format)
findings = []  # Populate with actual findings
for row_idx, finding in enumerate(findings, 2):
    for col_idx, value in enumerate(finding, 1):
        cell = ws.cell(row=row_idx, column=col_idx, value=value)
        cell.font = data_font
        cell.alignment = wrap_top
        cell.border = thin_border
    # Color risk level
    risk_col = 12
    risk_val = finding[11]
    if risk_val in risk_colors:
        risk_cell = ws.cell(row=row_idx, column=risk_col)
        risk_cell.fill = PatternFill(start_color=risk_colors[risk_val],
                                     end_color=risk_colors[risk_val], fill_type="solid")

ws.freeze_panes = "A2"
wb.save("external_input_audit.xlsx")
```

## 执行注意事项

1. **聚焦 C++ 源文件**（.cpp, .cc, .h, .hpp），默认排除 `ets` 和 `cj` 前缀的文件
2. **优先审计 IPC 密集模块**（services/abilitymgr, services/appmgr），它们的 Parcel 反序列化是主要攻击面
3. **关注 `frameworks/native/` 和 `services/` 目录**，这些是外部输入的入口点
4. **对于 NAPI 绑定层**（frameworks/js/napi/），重点检查 napi_get_value_* 获取的参数是否在持久化前校验
5. **每个发现必须有代码行号和具体的代码引用**，禁止凭空猜测
6. **优先报告 Confirmed 的 P0/P1 问题**，控制总数量保证信噪比
7. **引用代码使用格式**：`path/to/file.cpp:123`
8. **如需了解更细粒度的 C++ 持久化模式**，读取 `references/sink_patterns.md`
