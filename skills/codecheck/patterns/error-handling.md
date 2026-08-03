# 错误处理模式 (Error Handling)

> 所有 scanner 共享的错误处理缺陷模式词典。

---

## ERR-001 忽略返回值

**信号特征**
- 所有可能失败的 API 返回值未检查（`ReadString`/`stream.read`/`ReadBuffer`）。
- 嵌套解析/反序列化返回值未检查（`Unmarshalling`/`PublicReadFromParcel`）。
- 数据库写入/文件写入返回值未检查。
- `fopen` 返回值未检查即 `fread`。

**grep 线索**
```bash
rg -n "parcel\.Read|stream\.read|ReadBuffer" --type cpp
rg -n "Unmarshalling\(|PublicReadFromParcel\(" --type cpp
rg -n "database_->Insert|WriteToFile" --type cpp
rg -n "fopen" --type cpp   # 检查是否检查返回值
```

**典型后果**：使用失败结果/空数据继续执行，P0/P1。

**历史案例**：G14（`ReadFromParcelWantParamWrapper` 忽略返回值）、高影响清单 7.1/7.2/7.5。

---

## ERR-002 错误路径返回成功

**信号特征**
- 异常分支（如 `QueryAbilityInfo` 失败）打印日志后返回 `true` / `ERR_OK`，而非错误码。
- `switch` 的 `default`/`else` 分支对未知类型返回 true。
- 未知/未识别类型默认 fallthrough 为成功。

**grep 线索**
```bash
rg -n "return\s+true|return\s+ERR_OK" --type cpp   # 在错误分支
rg -n "default:" --type cpp   # 检查是否 fallthrough 成功
```

**典型后果**：异常路径影响正常流程、逻辑绕过，P0/P1。

**历史案例**：G14（`bundleMgrHelper->QueryAbilityInfo` 失败分支打印日志返回 `true`）、高影响清单 4.4/7.3/7.4。

---

## ERR-003 类型转换无异常兜底

**信号特征**
- `std::stoi`/`std::stol`/JSON 类型转换无 `try-catch`。
- 转换失败跨越 Native/IPC 边界抛异常。
- `optional.value()`/`variant get`/`vector::at()` 未捕获异常。
- `make_unique`/`std::string::resize` 等大分配未防 `bad_alloc`。

**grep 线索**
```bash
rg -n "std::stoi|std::stol|std::stoul" --type cpp
rg -n "throw" --type cpp
rg -n "\.at\(\)|\.value\(\)" --type cpp
```

**典型后果**：进程崩溃（DoS），P0。

**历史案例**：G3（`std::stoi` 未 try-catch 历史高发 DoS 点）、高影响清单阶段 2 未捕获异常。

---

## ERR-004 部分失败 / 未回滚

**信号特征**
- 成功/打开标志在所有子步骤成功前设置（`isOpen_ = true` 在解析失败后仍置 true）。
- 失败时留下半初始化/半打开对象。
- 拷贝/赋值未用 copy-and-swap（先 `clear()` 再分配）。
- 嵌套解析返回值未检查，中途失败已分配子容器未释放。
- 多表/多文件写入无原子性，部分失败无回滚。
- 批量写入第 K 条失败后前 K-1 条未回滚。

**grep 线索**
```bash
rg -n "isOpen_\s*=\s*true|isInitialized_\s*=\s*true" --type cpp
rg -n "clear\(\)" --type cpp
rg -n "return\s+ERR_OK" --type cpp   # 检查中间步骤是否都成功
```

**典型后果**：状态不一致、数据损坏，P0/P1。

**历史案例**：高影响清单 4.1-4.5（`ZipFile::Open` 解析失败仍置 `isOpen_=true`、`WantParams::operator=` 弱异常安全）。

---

## ERR-005 错误码语义不一致

**信号特征**
- 文档声明的错误码触发条件未真的触发。
- 同一函数不同失败路径返回的错误码不可区分。
- 用通用错误码掩盖具体失败原因（返回 INTERNAL/TIMEOUT 掩盖权限拒绝）。
- 文档说"返回 X"但实现返回 Y（语义反转）。
- 值类型参数文档说"为空返回 PARAM_INVALID"（值类型不可能为空）。
- 错误码比较写反（`==` vs `!=`），用 0 判断成功而接口返回负值。

**grep 线索**
```bash
rg -n "return\s+ERR_|return\s+\d+" --type cpp
rg -n "==\s*0|!=\s*0" --type cpp
```

**典型后果**：调用方误判失败/成功、掩盖真实问题，P1。

---

## ERR-006 异常跨边界

**信号特征**
- 异常跨越 Native/IPC/API 边界抛出（未在入口点捕获）。
- `-fexceptions` 模块未对 `std::bad_alloc` 做入口级防护。
- `at()` 和解析异常未在入口点附近捕获。

**典型后果**：进程崩溃，P0。

---

## ERR-007 防御性 double-free（误报识别）

**信号特征**
- `free(ptr); ptr = nullptr;` 再 free。

**判定**
- 检查两次 free 之间 ptr 是否被置 null → 有置 null → **推翻**（防御性写法，非 bug）。

> 此模式用于 refute 阶段识别误报。

---

## ERR-008 UAF 仅涉及日志打印（误报识别）

**信号特征**
- "释放后使用"仅在 `HILOG_INFO` 中读整数值。

**判定**
- 检查 use 点是否影响控制流或数据完整性 → 仅日志 → **降两级**。

> 此模式用于 refute 阶段识别误报。

---

## ERR-009 静态 bool 标志位无锁（误报识别）

**信号特征**
- 标志位用于一次性初始化，设置了就不再改变。

**判定**
- 检查标志位是否只在单线程初始化阶段设置 → 单次设置 → **降级为 P3**。

> 此模式用于 refute 阶段识别误报。
