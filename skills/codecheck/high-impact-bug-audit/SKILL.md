---
name: high-impact-bug-audit
description: Systematically audit code for high-impact bugs across ability_base or similar codebases. Use when asked to find, review, scan, investigate, or prioritize P0/P1 risks such as crashes, hangs, deadlocks, OOM, illegal memory access, data corruption, destructive side effects, resource leaks, state pollution, permission bypass, serialization/IPC bugs, malformed input handling, file/path/archive parsing bugs, lifecycle bugs, concurrency races, or other severe reliability/security defects.
---

# High Impact Bug Audit

## Purpose

Audit by final result impact, not by code smell alone. Prioritize issues that can cause crash, hang, OOM, data corruption, destructive side effects, persistent resource leaks, state pollution, permission bypass, or other high-impact service failures.

Use this priority formula:

```text
Priority = impact severity * triggerability * blast radius
```

Every finding must answer:

- What final result can this cause?
- Who or what can trigger it?
- How large is the impact once triggered?

## Workflow

1. **Build a module risk profile.** Summarize the module's main capability, external inputs, public/API/IPC/file/config entry points, resources owned, side effects, async/concurrent behavior, and likely P0/P1 outcomes.
2. **Run the core scan first.** Check null/invalid dereference, out-of-bounds access, UAF/lifetime, deadlock/hang, OOM/uncaught exception, deserialization, integer boundary, resource leak, partial failure/state pollution, ignored return values, and caller triggerability.
3. **Select capability-specific checks.** Use the module's actual capabilities to choose relevant checklist sections. Do not mechanically run every item when it does not apply.
4. **Confirm trigger paths.** For each P0/P1 candidate, prove or disprove whether UI, API, IPC, config, file, network, test, or other external input can reach it.
5. **Classify evidence.** Separate Confirmed, Likely, Suspicious, and Excluded items.
6. **Output high-signal results.** Lead with Confirmed/Likely P0/P1 findings sorted by priority. Put Suspicious items in follow-up. Record Excluded items only when useful to prevent duplicate review.

## Core Scan

Always check these for every module:

- Null pointer, invalid iterator, empty string/array indexing, unchecked `Query`/`Get`/`find` results.
- Bounds errors in `[]`, `at()`, pointer arithmetic, `memcpy`/`memmove`, buffer read/write, `offset + length`.
- Use-after-free, double-free, callback after close/dispose, raw pointer captured by async work.
- Infinite loop, recursion without depth limit, wait without timeout, lock-order deadlock, blocking work on critical threads.
- Uncaught exceptions across Native/API/IPC boundaries, `bad_alloc`, unbounded allocation, large input materialization.
- Parcel/JSON/string/binary deserialization and type conversion failures that continue execution.
- Integer overflow, underflow, truncation, negative-to-unsigned conversion, `size_t` to smaller integer writes.
- Leaked fd, mmap, socket, thread, timer, callback, observer, native reference, async work handle.
- Half-initialized state, success flag before all steps succeed, missing rollback on error paths.
- Ignored return values, unknown/default types returning success, swallowed errors.

## Capability Routing

Use the detailed checklist only for relevant capabilities:

| Capability | Read in reference |
| --- | --- |
| File, path, archive, binary format parsing | Categories 1, 2, 3, 5, 8, 9 |
| Parcel, JSON, string, parameter container | Categories 2, 4, 5, 7, 9 |
| mmap, fd, socket, native handle | Categories 2, 3, 6, 7 |
| Global cache, singleton, async, shared state | Categories 3, 4, 6 |
| Permission, security validation, code/resource loading | Categories 5, 7, 8, 9 |
| Pure data conversion or utility code | Categories 4, 5, 7, 9 |

Read `references/high-impact-checklist.md` when you need the full checklist, detailed search patterns, or category-specific review prompts.

## Evidence Levels

| Level | Meaning | Output handling |
| --- | --- | --- |
| Confirmed | Clear code evidence and trigger path | Report as finding with fix and test advice |
| Likely | Code risk is clear, trigger path needs more confirmation | Report as high-priority candidate |
| Suspicious | Pattern is concerning, result or triggerability unclear | Put in follow-up, do not overstate |
| Excluded | Protected by upstream condition, invariant, or caller constraint | Record exclusion reason when useful |

## Finding Format

Use this format for each Confirmed/Likely item:

```text
Title:
Location:
Priority:
Evidence level:
Result impact:
Trigger condition:
Blast radius:
Root cause:
Evidence:
Recommended fix:
Recommended tests:
```

When possible, state the trigger path concretely:

```text
Given input X through API Y, code path Z can cause result R.
```

## Output Template

When the audit request asks for Excel output, generate an `.xlsx` file using the following template.

### Excel Structure

| Column | Header | Description |
| --- | --- | --- |
| A | 文件路径 | Relative file path from repo root (e.g. `interfaces/kits/native/configuration/src/configuration.cpp`) |
| B | 行号 | Line number or range (e.g. `41`, `157-158`, `264-267`) |
| C | 问题概述 | One-line summary of the issue (max ~50 chars) |
| D | 问题详细描述 | Full description in Markdown: `### 问题描述` + `### 修复建议` + `### 影响` |
| E | 问题类型 | One of: 内存安全, 整数安全, 类型安全, 并发安全, 输入验证, 状态污染, 资源泄漏, 逻辑缺陷, 初始化安全, 错误处理, 封装违反, 信息泄漏, 性能, 代码质量 |
| F | 风险等级 | One of: **致命**, **严重**, **一般**, **提示** |

### Risk Level Definitions

| 风险等级 | Definition | Typical Issues |
| --- | --- | --- |
| **致命** | Direct crash, OOM, data corruption, or security exploit with clear trigger path | 空指针解引用, 缓冲区越界, UAF, 构造异常泄漏, 恶意输入可触发 UB |
| **严重** | High risk of failure under realistic conditions; data race, type mismatch, unvalidated cast, state pollution | 竞态条件, 序列化类型不对称, 枚举强转无验证, 反序列化不清空, 整数溢出绕过校验 |
| **一般** | Correctness risk or maintenance burden; incomplete validation, performance issue, inconsistent behavior | 输入验证不完整, 浮点精确比较, 递归锁冗余获取, 忽略返回值, 成员未初始化 |
| **提示** | Code quality, style, or low-impact issue; spelling, redundant code, minor encapsulation | 拼写错误, 冗余检查, 未使用 include, 日志文本不准确, 缺少 operator!= |

### Formatting Rules

- **Header row (row 1):** Bold font, fill color `#4472C4` (blue), text wrap enabled, vertical alignment center.
- **Data rows:** Calibri 11pt, text wrap enabled, vertical alignment top, thin borders on all cells.
- **Column widths:** A=55, B=12, C=45, D=90, E=15, F=10.
- **Row height:** 80 for data rows.
- **Freeze:** Freeze row 1 (freeze_panes = `A2`).
- **Sheet naming:** Use a unified sheet for all findings, optionally add per-module sheets named `<module> 模块问题`.
- **Sort order:** Sort by 风险等级 (致命 > 严重 > 一般 > 提示), then by file path.

### Python Generation Example

```python
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

wb = openpyxl.Workbook()
ws = wb.active
ws.title = "高影响问题清单"

# Styles
header_font = Font(bold=True)
header_fill = PatternFill(start_color="004472C4", end_color="004472C4", fill_type="solid")
data_font = Font(name="Calibri", size=11)
wrap_alignment = Alignment(wrap_text=True, vertical="top")
thin_border = Border(
    left=Side(style="thin"), right=Side(style="thin"),
    top=Side(style="thin"), bottom=Side(style="thin"),
)

# Column widths
col_widths = {1: 55, 2: 12, 3: 45, 4: 90, 5: 15, 6: 10}
for col_idx, width in col_widths.items():
    ws.column_dimensions[get_column_letter(col_idx)].width = width

# Headers
headers = ["文件路径", "行号", "问题概述", "问题详细描述", "问题类型", "风险等级"]
for col_idx, header in enumerate(headers, 1):
    cell = ws.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = Alignment(wrap_text=True, vertical="center")
    cell.border = thin_border

# Data rows (example)
issues = [
    ("interfaces/kits/native/uri/src/uri.cpp", "507-510",
     "Unmarshalling 返回裸指针，构造异常时内存泄漏",
     "### 问题描述\n`Uri::Unmarshalling()` 返回裸指针，若构造函数抛异常则内存泄漏。\n\n### 修复建议\n使用 std::unique_ptr 或 try-catch。\n\n### 影响\n所有 IPC 反序列化路径。",
     "资源泄漏", "致命"),
]

for row_idx, issue in enumerate(issues, 2):
    for col_idx, value in enumerate(issue, 1):
        cell = ws.cell(row=row_idx, column=col_idx, value=value)
        cell.font = data_font
        cell.alignment = wrap_alignment
        cell.border = thin_border

ws.freeze_panes = "A2"
wb.save("audit_issues.xlsx")
```

## Review Discipline

- Sort by priority, not file order.
- Treat P0/P1 candidates as high priority until triggerability is disproven.
- Downgrade only when impact or triggerability is clearly limited.
- Prefer fewer, better-supported findings over a long list of weak code smells.
- For code review requests, lead with findings and file/line references.
- For broad audits, include a brief module risk profile and a short list of follow-up scan areas.
