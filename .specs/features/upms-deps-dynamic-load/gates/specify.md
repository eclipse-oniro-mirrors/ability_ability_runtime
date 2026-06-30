# Gate — Stage 2 Specify（规格化阶段）

> item: `upms-deps-dynamic-load` | 阶段切换：Stage 2 → Stage 3
> complexity: **复杂**（Stage 2 由 standard 升级，见 design §0）
> 规则依据：SKILL.md「Stage 2 退出条件」+ 复杂层裁剪（design 全量+设计扩展、spec 全量+场景覆盖）。
> 取证范围：本轮 4 路全量依赖审计（逐依赖 文件:行号）+ 独占性取证（services/*/BUILD.gn 命中 + sa_profile 进程归属）+ 现有 DllWrapper/SRMS 先例 + deps_wrapper 机制核查。

## 一、规格化阶段门禁

| # | 检查项 | 结果 | 证据 / 理由 |
|---|--------|------|------------|
| S1 | design.md 关键决策（ADR）全量且含取舍 | ✅ 通过 | design.md §3 ADR-1~10，每条含决策+理由+取舍（框架/契约/守卫/分组/retrofit/死依赖/触发/共享库零收益/flag/独占门槛） |
| S2 | 与先例/现状差异澄清（不照搬） | ✅ 通过 | design §2 独占性矩阵 + deps_wrapper 核查（编译时条件编译，不可复用→新建框架）；与废弃 SA 卸载差异（ADR-7） |
| S3 | 全量依赖有源码取证 | ✅ 通过 | 4 路审计：rdb/data_share/fileuri/storage（A）、media/udmf/sandbox（B）、bundle/access/ability_base+deps_wrapper（C）、ipc/samgr/...核心+helper（D），逐依赖 文件:行号 |
| S4 | 独占性有取证（复杂层：内存收益依据） | ✅ 通过 | design §2：services/*/BUILD.gn 命中 + sa_profile（foundation 驻留 SA 180/182/183/185/501）→ bundle/access/sandbox/udmf 共享、media/storage/fileuri/broker 独占候选 |
| S5 | spec.md AC 全 WHEN/THEN 且可测 | ✅ 通过 | spec §3，3×P0 + 7×P1（含框架通用性/retrofit/独占门槛），每条含验证手段 |
| S6 | AC 与 proposal/design 可追溯 | ✅ 通过 | spec §7 可追溯矩阵（AC ↔ proposal §3.5/§3.9 ↔ design ADR ↔ 验证 ↔ 阶段） |
| S7 | Stage 1→2 不涉及项 carry-through | ✅ 通过 | design §6 表 7 项全部确认 |
| S8 | 兼容性已分析（行为/flag/进程/依赖/API） | ✅ 通过 | spec §6：对外 API 无变更；flag 语义不变；进程/SA/sa_profile 不变；原依赖仓不动 |
| S9 | 插件契约清晰（interface+C 工厂+守卫规约） | ✅ 通过 | design ADR-1/2/3 + spec §5：IDynamicFeature 基类 + 7 接口 + CreateFeature/DestroyFeature + Acquire RAII 守卫 + 不跨作用域持指针 |
| S10 | 场景覆盖（复杂层：各插件/各 URI 类型/卸载重载） | ✅ 通过 | spec AC-2/AC-3 覆盖 7 插件 × 各 URI 类型（media/docs/sandbox/分布式/UDMF Key）的卸载→重载强一致；AC-10 独占性逐插件实测 |
| S11 | 安全/稳定风险识别 + 缓解 | ✅ 通过 | design §7 R1~R8；R1(独占误判)/R2(UAF) 为高，由 ADR-10(实测门槛)+ADR-3(守卫活动计数) 缓解，AC-2/3/5/10 覆盖 |
| S12 | 关键不确定项有门槛/Fallback | ✅ 通过 | ADR-10/AC-10：独占性 /proc/maps 实测为 Phase 2 硬门槛，非独占 descoped；R3 共享库零收益已明示接受 |
| S13 | 复杂度裁剪一致（复杂：design 全量+扩展、spec 全量+场景） | ✅ 通过 | design（ADR-1~10+架构+独占矩阵+分组+分阶段）+ spec（10 AC+场景+契约+追溯）；复杂度升级与范围匹配 |

**结论：Stage 2 门禁全部通过（13/13，复杂层）。**

## 二、带入 Stage 3 的移交事项

1. execution-plan.md + task.md 按 design §5 + spec §4 分阶段拆任务（需求方调整：media→P1、broker→P2）：
   - **Phase 1**：共享接口头 + `DynamicFeatureManager`(h+cpp) + RAII 守卫 + 延时任务(ffrt) + **media 插件 .so（首个用例：C++ SDK 单例 + wrapper .so + C 工厂）** + 死依赖清理(~11，逐个 build 验证)；media 独占性(AC-10)作 P1 早期实测。
   - **Phase 2（过 AC-10 门槛）**：broker retrofit + storage/fileuri 插件 .so。
   - **Phase 3（模块化）**：identity/sandbox/udmf 插件 .so。
2. OQ-1 `target_release` 在 Stage 3 向 owner 确认。
3. **硬门槛（Stage 3/4 实测）**：
   - AC-10 独占性：`/proc/<foundation_pid>/maps` 逐插件确认；非独占 descoped 决策。
   - AC-2/AC-3 强一致 + 在途 IPC（ASAN）；AC-5 并发（ASAN/TSAN）。
4. 设计待定（Stage 3 定）：插件分组是否合并（7→更少）；共享接口头落点（inner_api vs services）；延时选型 ffrt（已倾向）。
5. 代码须遵循 OpenHarmony C++ 规范：hilog 用 `AAFwkLogTag::URIPERMMGR`；dlopen/dlclose 用 `dlerror()` 记录（与现有 DllWrapper 一致）；插件 .so 用 `ohos_shared_library` + 合适 sanitize（cfi 等，与 libupms 一致）。

## 三、审批状态

- Stage 2 产物（design.md / spec.md / gates/specify.md）已按复杂层重写，门禁 13/13 通过；范围经全量审计+独占性取证演进，需求方逐轮确认。
- **等待需求方批准进入 Stage 3**。未批准前不创建 execution-plan.md / task.md，不写任何代码（HARD-GATE）。
