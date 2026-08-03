# G3 反序列化缺"四件套"

> 来源：known-defect-patterns.md G3。

## 特征信号 / grep 线索

- `ReadFromParcel` / `Unmarshalling` / JSON 解析 / 自定义二进制解析中：
  - 从 parcel 读出的 int32/size/count 直接用于 `resize`/`new`/循环次数（无上限校验）。
  - 递归解析嵌套结构无深度计数。
  - 按外部 length 循环读取无总次数/超时限制。
  - `std::stoi`/`stol`、类型转换无 try-catch。
- 解析逻辑位于权限校验代码**之前**（未授权即可触发）。

## 历史案例

- `SkillExecuteResult::ReadFromParcel` 未校验 uriCount → IPC 致 foundation CPU/内存耗尽（鉴权前触发，任意三方 App 可打）。
- `InsightIntentExecuteResult::ReadFromParcel` int32 直传 `resize` → OOM。
- `ParseWantParams` 递归无深度限制 → 开机加载通知 want 时栈耗尽 crash（不开机风险）。
- 44/79 字节 JSON → `ParseWantParamsWithBrackets` 死循环 600s+。
- `ReadFromParcel` 缺容器大小上限 → 内存放大；`EncodeBase64(srcLen=0)` 堆越界。

## 检查点

- 每个解析入口逐一核对四件套：长度/数量上限、递归深度、循环上限、异常兜底。
- 解析调用点是否可能先于鉴权执行？
