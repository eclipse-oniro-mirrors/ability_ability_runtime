# 内存安全模式 (Memory Safety)

> 所有 scanner 共享的内存安全缺陷模式词典。每条目格式：编号 + 模式描述 + 信号特征 + grep 线索 + 触发条件 + 典型后果 + 历史案例引用。

---

## MEM-001 空指针解引用 / 无效迭代器

**信号特征**
- `ptr->member`、`*ptr`、`ptr[index]` 前无 `if (ptr != nullptr)` 校验。
- `map.find(...)->second`、`dynamic_cast` / `Query` / `Get` / `ReadParcelable` 返回值未判空直接使用。
- 空字符串/空数组的 `[0]`、`front()`、`back()`。

**grep 线索**
```bash
rg -n "->" --type cpp | rg -v "nullptr|if\s*\(|return|assert"
rg -n "find\(.*\)->second" --type cpp
rg -n "\.front\(\)|\.back\(\)|\[0\]" --type cpp
rg -n "ReadParcelable|iface_cast|QueryAbilityInfo|GetBundleInfo" --type cpp
```

**触发条件**
- 指针来自查找失败、转换失败、分配失败或无效外部 id。
- 上层传入的 null / 缺失字段 / 无效 id 到达该路径。
- 容器为空时未拦截即索引。

**典型后果**：进程崩溃（SIGSEGV），P0。

**历史案例**：G14（返回值与异常分支处理）、`ability_manager_stub.cpp` ReadParcelable 空指针。

---

## MEM-002 数组越界 / 缓冲区写越界

**信号特征**
- `vec[i]`、`arr[i]`、`str[i]` 索引未与 `size()` 校验。
- `memcpy` / `memmove` / `strcpy` / `strncpy` 长度参数失控。
- `strcpy_s` 误把源串长度当作目标缓冲区上限（真实案例：栈溢出）。
- `offset + length` 溢出后读写越界。

**grep 线索**
```bash
rg -n "memcpy|memmove|strcpy|strncpy|strcpy_s" --type cpp
rg -n "for\s*\(.*i\s*<=\s*.*size" --type cpp   # 注意 <= 而非 <
rg -n "\[i\]|\[index\]|\[offset\]" --type cpp
rg -n "data\s*\+\s*offset|startPos\s*\+\s*bufferSize" --type cpp
```

**触发条件**
- 索引在所有路径上未与 `size()` 校验。
- 负值隐式转 `size_t` 变成巨大值。
- `offset + length` 溢出。
- 空字符串/空数组未处理即索引。

**典型后果**：堆/栈越界读写，可导致 RCE，P0。

**历史案例**：G03（`EncodeBase64(srcLen=0)` 堆越界）、高影响清单 1.5/1.6。

---

## MEM-003 释放后使用 / 生命周期 / 回调后访问 (UAF)

**信号特征**
- `delete` / `free` 后继续使用同一指针。
- 异步回调捕获 `this` 或原始裸指针，对象销毁后回调仍执行。
- 监听器/观察者持有 Native 对象引用，注销不彻底。
- `Close` / `Dispose` 后未阻止后续操作使用已释放资源。
- 智能指针手动 `get()` 后 delete → double free。

**grep 线索**
```bash
rg -n "delete\s+\w|free\s*\(" --type cpp
rg -n "lambda.*\[this\]|\[=\]|\[&\]" --type cpp   # 异步捕获 this
rg -n "Close|Dispose|unsubscribe|RemoveObserver" --type cpp
rg -n "\.get\(\)" --type cpp   # 智能指针 get() 后可能 delete
```

**触发条件**
- 回调可能在所属对象销毁后执行。
- `close`/`dispose` 不能完全阻止后续回调。
- 重复释放导致 double-free。

**典型后果**：UAF / double-free，可导致 RCE，P0。

**历史案例**：G04（智能指针 double free）、G12（death recipient 竞争致保活）、高影响清单 3.6。

---

## MEM-004 整数溢出 / 截断 / 类型转换

**信号特征**
- `size_t` 转较小类型（`int32_t`/`int`）前未校验范围。
- unsigned 减法下溢（`a - b`，`a < b`）。
- 乘法/加法表达式溢出（`offset + size`、`len + adjust`、`nameSize + extraSize`）。
- 负数转 unsigned 变巨大值。
- 函数返回类型窄于实际值（`int32_t` 返回但成员是 `size_t`）。
- 有符号整数位运算（`~`、`&`、`|`、`^`、`>>`、`<<`）UB。

**grep 线索**
```bash
rg -n "static_cast<int>|static_cast<int32_t>|static_cast<uint32_t>" --type cpp
rg -n "static_cast<int>" --type cpp
rg -n "\w+\s*-\s*\w+" --type cpp   # unsigned 减法，需人工判断类型
rg -n "offset\s*\+\s*size|len\s*\+\s*adjust|nameSize\s*\+\s*extraSize" --type cpp
rg -n "WriteInt32.*size_t|WriteInt32.*\.size\(\)" --type cpp
```

**触发条件**
- size/offset/index/count 来自外部输入且无边界检查。
- 中间结果用 32 位承载后溢出再使用。
- 返回类型无法容纳实际值。

**典型后果**：越界读写、绕过范围校验、逻辑错误，P0/P1。

**历史案例**：G14（`request_id_util.cpp` 整数上溢）、高影响清单 9.1-9.4。

---

## MEM-005 未初始化变量

**信号特征**
- 变量声明后条件分支赋值，某路径未赋值即使用。
- 类成员变量未在构造函数初始化列表初始化。
- 条件初始化（`if (needData) data = GetData();`）后无条件使用。

**grep 线索**
```bash
rg -n "int\s+\w+;" --type cpp   # 无初始值的整型声明
rg -n "bool\s+\w+;" --type cpp
```

**触发后果**：读取未初始化值 UB，P0/P1。

**历史案例**：G14（返回值与异常分支处理）、未初始化变量 UB。
