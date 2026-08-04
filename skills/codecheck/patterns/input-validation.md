# 输入校验与数据流模式 (Input Validation & Data Flow)

> 所有 scanner 共享的输入校验缺陷模式词典。

---

## INP-001 反序列化缺"四件套"（历史最高发输入面，必查）

**信号特征**
- `ReadFromParcel` / `Unmarshalling` / JSON 解析 / 自定义二进制解析中：
  - 从 parcel 读出的 int32/size/count 直接用于 `resize`/`new`/循环次数（无上限校验）。
  - 递归解析嵌套结构无深度计数。
  - 按外部 length 循环读取无总次数/超时限制。
  - `std::stoi`/`stol`、类型转换无 try-catch。
- 解析逻辑位于权限校验代码**之前**（未授权即可触发）。

**四件套（必须同时具备）**
1. 容器/字符串**长度与数量上限**。
2. **递归深度上限**（嵌套结构解析）。
3. **循环上限/超时**。
4. **异常兜底**（stoi/类型转换 try-catch）。

**grep 线索**
```bash
rg -n "ReadFromParcel|Unmarshalling" --type cpp
rg -n "parcel\.Read|Parcel::Read|ReadParcelable|ReadString|ReadInt32" --type cpp
rg -n "resize\(|new\s+\w+\[|std::make_unique<.*\[" --type cpp
rg -n "std::stoi|std::stol|std::stoul" --type cpp
rg -n "ParseWantParams|ParseJson|nlohmann::json" --type cpp
```

**典型后果**：IPC 致 foundation CPU/内存耗尽、OOM、栈耗尽不开机、死循环，P0。

**历史案例**：G3（`SkillExecuteResult::ReadFromParcel` 未校验 uriCount、`InsightIntentExecuteResult` OOM、`ParseWantParams` 递归无限制不开机、44 字节 JSON 死循环 600s+、`EncodeBase64(srcLen=0)` 堆越界）。

---

## INP-002 SQL 注入

**信号特征**
- `sprintf(sql, ..., input)` 或字符串拼接 SQL。
- 无 PreparedStatement / ORM，直接 `ExecuteSql(rawSql)`。

**grep 线索**
```bash
rg -n "ExecuteSql|execSQL|Execute\(" --type cpp
rg -n "sprintf.*sql|sql\s*\+=|sql\s*=\s*\"" --type cpp
rg -n "ValuesBucket|DataShareValuesBucket" --type cpp
```

**触发条件**：外部输入构成 SQL 语句的一部分且未参数化。

**典型后果**：DB 篡改/泄露，P0/P1。

---

## INP-003 路径遍历 / Zip Slip

**信号特征**
- 外部输入拼入文件路径前无规范化（`realpath`/`canonicalize`与白名单前缀校验。
- ZIP 解压条目名未校验 `..`（Zip Slip）。
- 临时目录使用可预测路径 + 非独占创建（TOCTOU 竞争）。
- 安装/卸载流程残留中间目录可被注入。
- 同一函数存在"针对某个 POC 的特判修复"痕迹。

**grep 线索**
```bash
rg -n "fopen|ofstream|open\s*\(" --type cpp
rg -n "realpath|canonical|normalize" --type cpp
rg -n "\.\.\/|\.\.\\" --type cpp
rg -n "unzip|ExtractFile|ZipFile" --type cpp
```

**检查点**
- 路径处理是否收敛到统一规范化+白名单函数？散落的点位拼接全部上报。
- 修复方式审查：特判封堵按"可被绕过"上报。

**典型后果**：覆盖目标目录外任意文件，P0。

**历史案例**：G6（`FileUtils.unzipFile` Zip Slip、hnp 解压路径穿越两轮复发、临时目录竞争覆盖 `/data/system/`）。

---

## INP-004 命令注入

**信号特征**
- 外部输入传递给 `system()` / `popen()` / `exec*()`。
- 直接拼接命令字符串：`"sh -c '" + userCmd + "'"`。

**grep 线索**
```bash
rg -n "system\s*\(|popen\s*\(|execv|execl|execvp" --type cpp
rg -n "sh -c|/bin/sh|/bin/bash" --type cpp
```

**典型后果**：任意命令执行，P0。

---

## INP-005 格式字符串

**信号特征**
- 外部输入作为 `printf`/`sprintf`/`fprintf` 的 format 参数（非字面量）。
- 日志 format 含 `%{public}s` 且参数来自外部输入。

**grep 线索**
```bash
rg -n "printf|sprintf|fprintf" --type cpp
rg -n "HILOG_INFO|HILOG_WARN|HILOG_ERROR|TAG_LOG" --type cpp
```

**典型后果**：栈泄露/写越界，P0/P1。

---

## INP-006 日志注入 (CRLF)

**信号特征**
- 外部输入直接写入日志，未过滤 `\r` `\n`。
- 日志解析器可能被欺骗（伪造日志条目、注入伪造行）。

**grep 线索**
```bash
rg -n "HILOG_INFO|HILOG_WARN|HILOG_ERROR" --type cpp
rg -n "%\{public\}s" --type cpp
```

**典型后果**：日志伪造、审计绕过，P1/P2。

---

## INP-007 Null Byte 注入

**信号特征**
- `std::string` 含 `\0` 但按 `c_str()` 边界截断。
- C/C++ 字符串终止符被利用绕过校验。

**典型后果**：绕过路径/类型校验，P1。

---

## INP-008 反序列化炸弹 / 大内存分配

**信号特征**
- JSON/二进制数据无深度/大小限制直接解析。
- 基于外部输入大小分配堆内存（`new uint8_t[externalSize]`、`resize(externalSize)`）无上限。
- 一次性分配完整输出（非流式/分块）。
- 异常路径未释放已分配资源（无 RAII）。
- 拷贝构造/赋值未满足强异常安全（先 `clear()` 再分配）。

**grep 线索**
```bash
rg -n "new\s+uint8_t\[|new\s+char\[" --type cpp
rg -n "\.resize\(" --type cpp
rg -n "std::make_unique<.*\[" --type cpp
rg -n "nlohmann::json|cJSON" --type cpp
```

**检查点**
- 单条目和总体是否有内存上限（`maxAllowedSize`）。
- 是否优先流式/分块处理。
- 异常路径是否释放已分配资源。

**典型后果**：OOM、栈溢出，P0。

**历史案例**：高影响清单 2.1-2.6。

---

## INP-009 TOCTOU (Time-of-Check to Time-of-Use)

**信号特征**
- 先检查文件存在再打开（`access` + `fopen`）。
- 先 SELECT 再 UPDATE（无事务/无锁）。
- 先检查权限再执行（期间状态被改）。

**grep 线索**
```bash
rg -n "access\s*\(|stat\s*\(" --type cpp
rg -n "if\s*\(.*exists|if\s*\(.*find" --type cpp
```

**典型后果**：竞态绕过校验，P0/P1。

---

## INP-010 协议一致性 / 序列化端-反序列化端不对齐

**信号特征**
- JSON/序列化数据解析逻辑与传入格式不符。
- 序列化端与反序列化端对同一参数处理逻辑不一致（走私攻击面）。
- 类型混淆（持久化数据按错误类型解析）。

**grep 线索**
```bash
rg -n "ReadFromParcel|WriteToParcel|Marshalling|Unmarshalling" --type cpp
rg -n "static_cast<.*Parcel|static_cast<.*MessageParcel" --type cpp
```

**典型后果**：内存溢出、参数走私、类型混淆 RCE，P0。

**历史案例**：G1（CLI-SA 与沙箱间序列化/反序列化参数处理不一致 → 命令行参数走私）。
