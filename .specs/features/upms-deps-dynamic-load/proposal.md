# Proposal — UPMS 依赖库动态加载/卸载

> id: `upms-deps-dynamic-load` | type: feature | complexity: 复杂（Stage 2 升级） | profile: none
> 状态：**Stage 2 范围演进后重写（见 §3.9 + design §0），待批准进入 Stage 3**
> 本文件三合一：原始需求 → 澄清记录 → 需求基线（一份文档不断生长）。
> 替代已废弃的 `upms-dynamic-unload`（SA 实体级卸载，经验证不可行）。
> **注**：§3.1~3.8 反映 Stage 1 基线（4 库 dlopen/standard）；Stage 2 全量审计 + 独占性取证后范围演进为「通用插件框架 + 抽 9 依赖 + retrofit + 死依赖清理、complexity→复杂」，详见 §3.9 与 [design.md](design.md) §0/§2/§3。

---

## 一、原始需求 (Raw Request)

为 UPMS（Uri Permission Manager Service，SA 183，库 `libupms.z.so`，运行于 `foundation` 进程）降低空闲内存。

前置可行性分析（本地笔记，未入库）已**否决 SA 实体级动态卸载**路径（即已废弃的 `upms-dynamic-unload` 方案）——核心障碍是 SA 级卸载机制对该服务不可行。需求方在确认 SA 卸载不可行后，将方向转向**依赖库粒度**：把 UPMS 依赖的较重、间歇使用的库（rdb、data_share、media_library、udmf）封装成可运行时 dlopen/dlclose 的模块，在 UPMS 空闲时卸载这些依赖 so、被需要时再加载，从而把空闲期内存还给 `foundation` 进程。UPMS 本体（轻壳）常驻不动。

---

## 二、澄清记录 (Clarification Record)

逐轮问答（每次一问，多选优先）。每轮记录：问题 / 选项 / 决策 / 理由。

### 轮 0：清理范围（前置）
- **问**：废弃旧方案时"清理前面已完成的"具体指？
- **选项**：只重做 .specs 文档 / **连代码一起推倒重来** / 旧的归档另开新特性
- **决策**：**连代码一起推倒重来**。本地 `git reset --hard HEAD~2` 抹除旧 SA 级实现（commit `e06439095f`）与旧 .specs（commit `6d069578a0`）；远端 `own/upmsop1` 暂留作备份，未 force-push。

### 轮 1：重做驱动力
- **问**：重做 UPMS 自卸载的核心驱动力？
- **选项**：老方案路径被否目标不变 / **需求范围变了** / 只是规范重走 SDD
- **决策**：**需求范围变了**。卸载机制/对象本身改变；特性目标仍是降内存，但已验证 SA 级动态卸载不可行，转向依赖库动态加载/卸载。

### 轮 2：需求变化维度
- **问**：需求范围具体哪些方面变了？
- **决策**：卸载机制/对象本身变了——从"卸载 UPMS 服务实体"改为"动态加载/卸载依赖库，把依赖库打包封装成可动态加载/卸载的 so"。

### 轮 3：目标依赖库
- **问**：把哪些依赖库封装成可 dlopen/dlclose 的 so？
- **决策（多选）**：`relational_store`(native_rdb + native_dataability)、`data_share`(datashare_consumer)、`media_library`(media_permission_helper)、`udmf`(udmf_client)。即 UPMS 全部非核心、间歇使用的重依赖（见 [BUILD.gn:69-95](../../../services/uripermmgr/BUILD.gn#L69-L95) external_deps）。

### 轮 4：加载/卸载触发
- **问**：重依赖 so 何时 dlopen / dlclose？
- **选项**：**空闲延时触发** / 按操作触发（用完即卸）/ 内存压力信号 / 组合
- **决策**：**空闲延时触发**——UPMS 空闲 N 秒后 dlclose 重依赖；下次有 IPC 需要时 dlopen 恢复。（思路与废弃方案"延时"同源，但作用对象从 SA 本体下沉到依赖库粒度。）

### 轮 5：涉及仓范围
- **问**：方案涉及哪些仓？
- **选项**：**单仓（仅 ability_runtime）** / 跨仓（改依赖库本身）/ 还不确定
- **决策**：**单仓**。UPMS 把对目标库的调用收拢到 dlopen/dlclose 包装层，运行时 dlopen 现有系统 so；依赖库本身不动。

### 轮 6：内存验收线
- **问**："降内存"的验收线怎么定？
- **选项**：硬性数字 / **不设硬数字，证释放即可** / Stage 2 测完再定
- **决策**：**不设硬数字，证释放即可**——只要证明 dlclose 后这些库占用的 PSS 确实被释放、差值可稳定复测。

### 轮 7：状态一致性
- **问**：卸载→重载周期内 URI 授权状态必须满足什么？
- **选项**：**强一致：零丢失零错授权** / 允许重建最终一致 / 还没定
- **决策**：**强一致**——周期前后授权状态完全一致；卸载期间到达的 IPC 必须先 dlopen 加载再正确处理。

### 理解确认（基线冻结前）
已向需求方输出完整理解草稿（核心 / 范围含 / 排除项 / 涉及仓 / 复杂度 / P0-P1 AC / Stage 2 开放项），需求方回复 **"Y"**，授权写入基线草案。

---

## 三、需求基线 (Baseline)

### 3.1 核心目标
给 UPMS 增加依赖库粒度的动态加载/卸载能力：UPMS 本体常驻不动，把 rdb/data_share/media_library/udmf 等重依赖封装成可运行时 dlopen/dlclose 的模块；空闲延时（≥N 秒无活动）dlclose 这些依赖 so，下次 IPC 需要时 dlopen 恢复。在**授权状态强一致**前提下，把空闲期内存还给 `foundation` 进程。

### 3.2 范围 (In Scope)
1. 新增"依赖动态加载管理器"：统一封装对 rdb/data_share/media_library/udmf 的 dlopen/dlclose + 调用代理。
2. 改造 UPMS 对目标库的调用，收拢到该管理器（替换常规链接/直接调用为按需动态加载通道）。
3. 空闲延时触发：空闲计时（仿 EventHandler 延时任务）→ 到期 dlclose；任意 IPC/活动到来取消计时并重挂；卸载期间到达的请求触发 dlopen 后再处理。
4. 同步保护：dlclose/dlopen 与并发 IPC 之间的锁/生命周期管理，避免 use-after-free。
5. 单元测试：覆盖"空闲→卸载""卸载期间 IPC→加载→正确处理""周期前后授权一致""并发安全"。

### 3.3 排除项 (Out of Scope)
- ❌ 不做 SA 实体级动态卸载（已验证不可行）。
- ❌ 不卸载核心库（ipc/samgr/safwk/access_token/bundle_framework/c_utils/eventhandler/hilog 等）——服务存活必需。
- ❌ 不改依赖库本身（relational_store/data_share/media_library/udmf 仓不动）——纯 ability_runtime 内包装。
- ❌ 不绑定具体 MB 内存数字（定性验收：证 PSS 释放即可）。
- ❌ 不改授权/撤销主链路语义（仅把对目标库的调用改走动态加载通道）。

### 3.4 涉及仓 / 子系统
**单仓** `ability_runtime`：
- `services/uripermmgr/`（依赖加载管理器、调用收拢、空闲触发、同步保护）
- 可能涉及 `services/uripermmgr/BUILD.gn`（目标库从常规链接改为按需 dlopen 的链接方式调整）

### 3.5 验收标准 (AC，WHEN/THEN，可测)
**P0**
- **AC-1 内存释放**：WHEN UPMS 持续空闲 ≥ N 秒 THEN 主动 dlclose 目标依赖 so，且 hidumper/hiprofiler 测得 UPMS PSS 较卸载前下降，下降量与被卸载库贡献相符、可稳定复测。
  - 验证：卸载前后 `hidumper -s 183` 或 hiprofiler 取 PSS，对比差值。
- **AC-2 状态强一致**：WHEN 发生一次"空闲卸载→按需重载"完整周期 THEN 周期前后 URI 授权状态完全一致（零丢失、零错授权）。
  - 验证：卸载前 Grant 一批 URI → 触发卸载 → 触发重载 → Check/Revoke 结果与卸载前完全一致。
- **AC-3 在途 IPC 安全**：WHEN 卸载期间（依赖 so 已 dlclose）有 IPC 到达 THEN 触发 dlopen 恢复后被正确处理，不 crash/不报错/不丢数据。
  - 验证：单测模拟"卸载态下注入 IPC"，断言结果正确且无 crash。

**P1**
- **AC-4 功能不回归**：WHEN 引入动态加载机制 THEN UPMS 全部对外接口（Grant/Revoke/Check/GrantByKey/Clear 等）行为与改造前一致，现有 UT 全过。
  - 验证：`run -t UT -tp ability_runtime`（uripermmgr 相关套）通过。
- **AC-5 并发安全**：WHEN dlclose 与并发 IPC 同时发生 THEN 无 use-after-free/crash/死锁。
  - 验证：单测并发场景 + ASAN/TSAN（如可用）。
- **AC-6 可观测**：WHEN 卸载/加载事件发生 THEN 有 hilog/hisysevent 记录。
  - 验证：触发卸载/加载后查日志确认事件上报。

### 3.6 约束与风险
- **dlclose 收益不确定性（已知）**：目标库可能被 `foundation` 内其它 SA 共享、`dlclose` 不保证物理释放、allocator 持有 vmem——本需求交付的是"能力 + 证 PSS 释放"，RSS 实测降幅属验证观测项，非硬性门槛（与废弃方案同一已知风险）。
- **状态强一致成本（命门）**：rdb 持有 RdbStore 连接句柄与缓存，dlclose 前必须正确关闭/释放所有连接，dlopen 后重建——这是 AC-2/AC-3 的实现前提，Stage 2 必须设计清楚连接生命周期。
- **包装层性能**：调用走 dlopen 句柄 + 函数指针/代理有微小开销；需评估高频路径（如 Check）影响。
- **空闲判定与误卸载**：N 与"空闲"定义需避免在持有活跃授权/在途操作时误卸载（与废弃方案的安全约束同源）。

### 3.7 不涉及项确认表
| 项 | 是否涉及 | 说明 |
|----|---------|------|
| SA 实体级动态卸载 | N/A | 已验证不可行，本项替代方向 |
| 核心库卸载 | N/A | ipc/samgr/safwk/access_token 等服务存活必需 |
| 跨仓 / 改依赖库本身 | N/A | 单仓，依赖库不动 |
| 具体内存 MB 量化目标 | N/A | 定性验收：证 PSS 释放即可 |
| 授权/撤销主链路语义改写 | N/A | 仅调用通道改走动态加载 |
| 新增对外 API | N/A | 无 |
| 授权账本"空才卸载"约束 | N/A | 本项不依赖账本为空（与废弃方案不同） |

### 3.8 待解决问题 (Open Questions，进入 Stage 2 前澄清)
| # | 问题 | owner | 处理时机 |
|---|------|-------|---------|
| OQ-1 | `target_release` 归属哪个版本？ | owner | Stage 2 起，可暂置 |
| OQ-2 | "封装成 so"的具体结构：新建单一包装 so / 每 dep 独立 so / 包装层 + dlopen 现有系统 so？ | design | Stage 2 design.md |
| OQ-3 | 空闲阈值 N 默认值？是否可配置？"空闲"精确判定（无 IPC / 无活跃授权 / 无在途操作）？ | design | Stage 2 design.md |
| OQ-4 | UPMS 当前对 rdb/data_share 等的实际用法调研（连接生命周期、是否常驻句柄、调用频度）→ 决定 dlclose 前清理动作与高频路径影响 | design | Stage 2 design.md（需读源码） |
| OQ-5 | dlopen 后如何调用目标库——函数指针表 / 抽象接口 + 实现转发 / 现有 client SDK 直接 dlopen？ | design | Stage 2 design.md |
| OQ-6 | 卸载/加载同步原语选型（mutex/RWLock/原子状态机）？ | design | Stage 2 design.md |

### 3.9 范围演进（Stage 2 全量依赖审计 + 独占性取证后，需求方逐轮确认）

Stage 1 基线（§3.1~3.8）假设"封装 rdb/data_share/media_library/udmf 做 dlopen/dlclose"。Stage 2 深入调研推翻/细化：

1. **rdb/data_share 是死依赖**（UPMS 零调用）→ 改为 BUILD.gn 清理，且发现共 ~11 个死依赖。
2. **sandbox_manager 是高频核心**（非候选）；**deps_wrapper 是编译时条件编译、不可复用**。
3. **独占性决定内存收益**：bundle/access_token/sandbox/udmf/ability_manager 与 abilitymgr/appmgr 同 foundation 进程共享 → UPMS 侧 dlclose 零近期收益；media/storage/fileuri/broker 为独占候选。
4. **机制升级为通用插件框架**：interface 基类 + 子类在独立 .so + 管理器 dlopen 调 C 工厂创建对象返回基类指针 + 加载状态 + 空闲 dlclose。

**最终范围（需求方确认）**：建通用插件框架；抽取 9 个依赖（独占候选 media/storage/fileuri + 共享库 bundle/access_token/sandbox/udmf/ability_manager，共享库为模块化接受零近期收益）+ retrofit broker + 死依赖清理。**complexity: standard → 复杂**。

详见 [design.md](design.md) §0（演进记录）/§2（独占性矩阵）/§3（ADR-1~10）/§4（架构）。本节为准基线，§3.1~3.8 视作 Stage 1 历史快照。
