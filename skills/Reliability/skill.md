---
name: meta-ability-reliability
description: 元能力可靠性skill。在元能力子系统（Want/InsightIntent）代码开发或设计阶段，自动提醒可靠性关注点，包括故障模式和资源限制风险。触发场景：Want代码修改、InsightIntent代码修改、want基础库开发、意图执行管理代码、序列化反序列化开发、fd传递代码修改、元能力可靠性设计。当用户开发或修改Want/InsightIntent相关代码时，此skill基于故障模式库和资源限制规格主动提示可靠性风险，并路由到对应模块级skill。
allowed-tools: [Read, Glob, Grep, Bash, Write, Task, question]
metadata:
  tags: [reliability, meta-ability, want, insight-intent, fault-mode, resource-limit]
---

# 元能力可靠性skill

## 使用场景

当用户在元能力子系统进行以下活动时使用此skill：

- **Want代码开发**：修改want基础库代码（序列化、fd传递、自定义参数等）
- **InsightIntent代码开发**：修改意图执行管理代码
- **可靠性设计**：编写元能力需求设计文档的可靠性设计章节

## 核心功能

基于元能力子系统故障模式库和资源限制规格，在开发/设计时主动提醒可靠性关注点，并根据模块路由到对应的模块级skill。

### 数据来源

故障模式和资源限制规格内嵌于各模块级skill，本skill汇总跨模块关注点：

| 数据 | 位置 |
| --- | --- |
| Want 故障模式（3条）与资源限制（14条） | `want/skill.md` |
| InsightIntent 故障模式（1条）与资源限制（2条） | `insight-intent/skill.md` |
| 跨模块关键关注点（AMS/APPMS/Want） | 本文件"关键可靠性关注点"章节 |

注：当前仅Want/InsightIntent模块完成入库，其余模块随子skill建设逐步补充。

### 模块路由

根据用户涉及的代码模块，路由到对应的模块级skill：

| 模块 | 子skill路径 | 代码路径 | 故障模式 | 资源限制 |
| --- | --- | --- | --- | --- |
| Want | want/skill.md | ability_base + abilitymgr want相关 | 3条(I-04,F-06,数据注入) | 14条(ability_base 11条 + ability_runtime 3条) |
| InsightIntent | insight-intent/skill.md | services/abilitymgr/src/insight_intent/ | 1条(D-06) | 2条(执行记录/IPC回传) |

## 工作流程

### 步骤1：识别涉及的模块

根据用户的代码路径和请求内容，判断涉及哪个模块：

1. **代码路径匹配**：
   - `ability_ability_base` / `want.cpp` / `want_params.cpp` → Want模块
   - `insight_intent/` → InsightIntent模块

2. **关键词匹配**：
   - 序列化/fd传递/WantParams → Want
   - 意图/Intent → InsightIntent

3. **多模块**：如果同时涉及Want和InsightIntent两个模块，分别路由到对应模块级skill

### 步骤2：路由到模块级skill

读取对应模块的子skill文件（`want/skill.md` 或 `insight-intent/skill.md`），执行模块级可靠性分析。

如果未找到对应模块级skill，使用本文件"关键可靠性关注点"章节的跨模块故障模式进行分析。

### 步骤3：输出可靠性关注点

由模块级skill输出：
1. **故障模式**：该模块的已知故障模式
2. **资源限制**：该模块的资源限制规格
3. **代码级关注点**：与当前开发任务相关的可靠性风险
4. **设计建议**：针对当前需求/代码变更的可靠性设计补充

## 关键可靠性关注点（跨模块）

### 1. 开机启动事件循环积压（I级 - 不开机）
- **风险**：系统缺少launcher/oobe应用或bms查询接口bug导致死循环，设备反复重启
- **关注模块**：AMS开机启动流程
- **设计要求**：开机启动需有循环退出条件和超时保护；包对账机制作为可信恢复

### 2. 隐式匹配劫持（I级 - 安全+可靠性）
- **风险**：三方应用通过隐式匹配劫持系统应用能力
- **关注模块**：AMS skill匹配逻辑
- **设计要求**：系统应用隐式匹配需有权限校验和优先级控制

### 3. 同步IPC阻塞（I级 - 设备重启）
- **风险**：同步IPC访问非关键进程，对端阻塞导致AMS卡死
- **关注模块**：AMS所有同步IPC调用点
- **设计要求**：所有同步IPC必须设置超时；避免对非关键进程使用同步IPC

### 4. Want序列化/反序列化（I级 - 应用拉起失败）
- **风险**：湖内外序列化格式不匹配导致应用拉起失败
- **关注模块**：Want基础库
- **设计要求**：序列化格式需版本兼容；递归深度限制100层

### 5. fd泄漏（II级 - 资源泄露）
- **风险**：Want fd传递路径fd泄漏
- **关注模块**：Want基础库fd传递
- **设计要求**：fd传递需有完整的生命周期管理，异常路径必须关闭fd

### 6. 进程资源耗尽（I级 - 资源泄露）
- **风险**：ams拉起进程过多导致系统资源耗尽
- **关注模块**：APPMS进程管理
- **设计要求**：进程数量限制（缓存进程64、渲染进程40等）；常驻重启限制

## 注意事项

- 模块级skill提供更精确的可靠性关注点，本skill作为元能力子系统的入口和跨模块关注点汇总
- 与通用代码可靠性评审skill(sd-code-dfr-reliability-review)互补：本skill聚焦元能力已知故障模式和资源限制，通用skill聚焦代码构造级别风险
- 11条故障模式中I级9条（涉及不开机/设备重启），必须优先关注
- 资源限制规格中的"静默截断"和"静默丢弃"行为需特别关注，可能导致数据丢失且重启无法恢复
