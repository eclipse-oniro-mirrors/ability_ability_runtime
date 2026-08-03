# 资源生命周期模式 (Resource Lifecycle)

> 所有 scanner 共享的资源生命周期缺陷模式词典。

---

## RES-001 资源泄漏（fd / mmap / socket / 线程 / timer / callback / observer / native ref）

**信号特征**
- `fopen` / `open` 后异常路径未 `fclose` / `close`。
- `mmap` 无对应 `munmap`。
- `new` 后 return 错误未 `delete`。
- 异常路径锁未释放。
- 回调对象在超时/失败路径未清理。
- 嵌套容器反序列化中途失败时已分配的子容器未释放。
- `Variant_Clear` 未覆盖所有类型分支（IPC proxy/stub 类型常被遗漏）。
- 异步任务句柄、监听器/观察者、JS 回调引用未注销。

**grep 线索**
```bash
rg -n "fopen|open\s*\(" --type cpp
rg -n "mmap\s*\(" --type cpp
rg -n "new\s+\w+\s*\(|new\s+\w+\[" --type cpp
rg -n "lock_guard|unique_lock" --type cpp   # 检查异常路径
rg -n "AddCallback|RegisterObserver|AddListener" --type cpp
```

**检查点**
- 每次成功获取后，是否在成功**和错误路径**上都有释放？
- 泄漏是否每次调用都重复发生？→ OOM / 句柄耗尽 / 线程耗尽。
- 不同模式/类型的对象生命周期规则是否一致？

**典型后果**：持续性泄漏导致 OOM / 句柄耗尽，P1。

**历史案例**：高影响清单 3.1-3.6、G4（NAPI 引用泄漏）。

---

## RES-002 析构函数资源未释放

**信号特征**
- 析构函数未释放本类拥有的所有资源（fd / mmap / 动态分配内存）。
- RAII 包装不完整。

**grep 线索**
```bash
rg -n "~\w+\s*\(" --type cpp
rg -n "fopen|open\s*\(" --type cpp   # 检查对应析构是否 close
```

**典型后果**：资源泄漏，P1。

**历史案例**：高影响清单 2.6（析构未释放）。

---

## RES-003 虚析构函数缺失

**信号特征**
- 通过基类指针 `delete` 派生类对象，但基类析构非虚 → 只调基类析构，派生资源泄漏。

**grep 线索**
```bash
rg -n "class\s+\w+\s*{" --type cpp   # 检查是否有 virtual ~
rg -n "delete\s+\w" --type cpp
```

**典型后果**：资源泄漏，P1。

---

## RES-004 对象切片

**信号特征**
- 派生类对象按值赋值/传递给基类对象 → 切片，损害多态。
- 基类拷贝/移动构造和赋值未声明为非 public 或 delete。

**grep 线索**
```bash
rg -n "=.*\w+\(.*\)" --type cpp   # 值传递
```

**典型后果**：多态失效、状态不一致，P1/P2。

---

## RES-005 移动语义安全

**信号特征**
- 移动构造/赋值未将源对象资源正确重置（指针未置 nullptr）。
- 被 move 后的对象不可被正常析构。
- 依赖已被 move 对象的值。

**grep 线索**
```bash
rg -n "&&" --type cpp   # 移动构造/赋值
rg -n "std::move" --type cpp
```

**典型后果**：double free / UAF，P0/P1。

---

## RES-006 三/五/零法则违反

**信号特征**
- 声明了自定义析构/拷贝/移动其中之一，但未声明其余 → 非预期行为。
- 应优先遵循零法则（全部用默认）或五法则（全声明）。

**典型后果**：浅拷贝、资源泄漏，P1/P2。

---

## RES-007 成员初始化缺失

**信号特征**
- 类成员变量未显式初始化（声明时或构造函数初始化列表）。
- 读取未初始化成员 UB。

**grep 线索**
```bash
rg -n "class\s+\w+.*\{[^}]*\}" --type cpp   # 检查成员
```

**典型后果**：UB，P0/P1。

---

## RES-008 类型转换安全

**信号特征**
- `reinterpret_cast` 不相关类型转换。
- `const_cast` 移除 const/volatile（导致 UB）。
- `iface_cast` 误用（历史大批量问题，单模块 500+ 处）。

**grep 线索**
```bash
rg -n "reinterpret_cast|const_cast" --type cpp
rg -n "iface_cast" --type cpp   # 全库统计其误用模式
```

**典型后果**：UB、内存损坏，P0/P1。

**历史案例**：G14（`iface_cast` 返回值未判空）、类型转换 UB。

---

## RES-009 进程退出时资源泄漏（误报识别）

**信号特征**
- 静态分配的资源在进程退出时未释放。

**判定**
- 泄漏点在 `exit()` 路径上 → OS 自动回收 → **降级为 P3**。
- 非退出路径的泄漏维持原等级。

> 此模式用于 refute 阶段识别误报，不作为正式发现上报。

---

## RES-010 回调泄漏但数量有限（误报识别）

**信号特征**
- 标注"回调未清理导致泄漏"，但回调总数有上限（如最多 3 个）。

**判定**
- 计算实际泄漏上限，上限小 → **降级**。

> 此模式用于 refute 阶段识别误报。
