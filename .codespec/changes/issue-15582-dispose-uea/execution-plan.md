# 执行计划：嵌入式元服务隐私弹框改为模应用体验

## 1. 任务编排概览

### 1.1 任务依赖图

```
                    T1: Want 常量定义
                          │
                ┌─────────┼─────────┐
                ↓                   ↓
        T2: IPC 接口扩展         T4: ModalCallback 新类
        (IAbilityScheduler)     (EmbeddablePrivacyModalCallback)
                │                   │
                ↓                   │
        T5: 进程内 C++ 接口         │
        (UIExtension +              │
         UIExtensionContext)        │
                │ ←─────────────────┘
                ↓
        T3: AMS 入口
        (AbilityRecord + DisposedObserver)
                │
                ↓
        T6: BUILD.gn 更新
                │
                ↓
        T7: 单元测试
                │
                ↓
        T8: 验证（编译 + 现有测试回归）
```

### 1.2 任务清单

| ID | 任务 | 类型 | 依赖 | 预计代码量 |
|----|------|------|------|-----------|
| T1 | Want 常量定义 | 新增 | 无 | ~15 行（1 文件） |
| T2 | Inner API: IAbilityScheduler + Proxy + Stub | 新增 | T1 | ~80 行（3 文件改） |
| T3 | AMS 侧: AbilityRecord + DisposedObserver 入口 | 新增 | T1, T2 | ~70 行（3 文件改） |
| T4 | 新类: EmbeddablePrivacyModalCallback | 新建文件 | T1 | ~120 行（2 新文件） |
| T5 | 进程内 C++: UIExtension + UIExtensionContext | 新增 | T1, T4 | ~90 行（4 文件改） |
| T6 | BUILD.gn 更新 | 配置 | T4 | ~2 行（1 文件改） |
| T7 | 单元测试 | 新增 | T2-T5 | ~200 行（2 测试文件扩展/新建） |
| T8 | 验证 | 验证 | T1-T7 | 编译 + 现有测试 |

### 1.3 执行顺序

**阶段 A（基础）**：T1 → 并行 [T2, T4]
**阶段 B（应用进程侧）**：T5（依赖 T2, T4）
**阶段 C（AMS 侧）**：T3（依赖 T2）
**阶段 D（构建与测试）**：T6 → T7 → T8

---

## 2. 关键设计决策回顾

实施时必须遵守 design.md 中的 7 个 ADR，重点：

- **ADR-1**：在 `ExecuteUIExtension` 开头**新增独立分支0**，**不修改**现有三条分支代码
- **ADR-2**：新 IPC code `CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION` 必须追加在 enum **末尾**（不插入中间）
- **ADR-7**：所有新增项使用 `EmbeddablePrivacy` 前缀，与现有 `Modal` 命名隔离

---

## 3. 任务规格索引

每个任务的详细规格见 `task.md`，按 T1-T8 编号。

---

## 4. 交接与验证

### 4.1 单任务交接标准

每个任务完成后必须满足：
1. 代码符合 design.md 对应 ADR
2. 命名遵循 ADR-7（`EmbeddablePrivacy` 前缀）
3. 包含必要的错误日志（`TAG_LOGE`/`TAG_LOGI`，使用正确 tag）
4. 现有代码逐行不变（用 `git diff` 验证现有函数无修改）
5. 编译通过（任务全部完成后统一验证）

### 4.2 整体验证（T8）

```bash
# 1. 编译验证
./build.sh --product-name <product> --build-target ability_runtime

# 2. 现有测试回归
run -t UT -ts ability_manager_service_first_test
run -t UT -ts ability_manager_service_third_test
run -t UT -ts disposed_observer_test
run -t UT -ts ui_extension_modal_callback_test

# 3. 新增测试执行（T7 完成后）
run -t UT -ts embeddable_privacy_modal_callback_test
run -t UT -ts disposed_observer_test   # 扩展用例
```

### 4.3 验收对照

完成 T1-T8 后，对照 spec.md §5 的 AC1-AC14 逐项验证，所有 AC 必须通过。

---

## 5. 风险与缓解（实施阶段）

| 风险 | 缓解 |
|------|------|
| IPC code enum 顺序影响序列化兼容 | T2 中严格追加在末尾，不插入中间 |
| AbilitySchedulerStub 子类（UIExtension 侧）未实现新虚方法导致纯虚类 | T5 同步实现 UIExtension::CreateEmbeddablePrivacyUIExtension |
| 新增 ModalCallback 类的 weak_ptr 生命周期 | T4 严格借鉴 UIExtensionModalCallback 结构 |
| `EmbeddablePrivacyModalCallback` 头文件路径引入循环依赖 | T4 中仅前向声明 UIExtensionContext，include 放 .cpp |
| `IsUIExtensionExist` 复用导致 sessionId 冲突 | T5 中 uiExtensionMap_ 是同一 map，需评估是否共用——若共用，新链路与旧链路 sessionId 由 ArkUI 保证唯一 |

---

## 6. 实施完成定义 (DoD)

- [ ] T1-T7 所有代码已写入
- [ ] T8 编译通过
- [ ] 现有所有单元测试通过（回归无回归）
- [ ] 新增单元测试通过
- [ ] `git diff` 验证现有函数体逐行不变
- [ ] AC1-AC14 全部通过
- [ ] review.md 完成统一审查
