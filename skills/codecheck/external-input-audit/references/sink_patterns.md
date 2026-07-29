# ability_runtime 持久化模式参考 (Sink Patterns Reference)

以下列出 ability_runtime 代码库中常见的持久化锚点模式，用于 Step 1 搜索阶段的精确匹配。

## 1. 数据库操作 (RDB / SQLite)

### 常见 API

```cpp
// RDB Store 操作
ValuesBucket bucket;
bucket.PutString(key, value);
bucket.PutInt(key, value);
int result = rdbStore_->Insert(tableName, bucket);
int result = rdbStore_->Update(values, predicates, args);
int result = rdbStore_->Delete(predicates, args);
std::shared_ptr<ResultSet> result = rdbStore_->Query(predicates, columns);

// 原生 SQL
rdbStore_->ExecuteSql(sql);
rdbStore_->Execute(rawSql, args);
```

### 搜索模式

```bash
rg -n "rdbStore_|RdbStore|ValuesBucket|resultSet|Insert\(|Update\(|Delete\(|ExecuteSql|Query\(" --type cpp -l
```

## 2. 文件写入

### 常见 API（能力侧）

```cpp
// C 标准库
FILE* fp = fopen(path.c_str(), "w");
fwrite(data, 1, len, fp);
fprintf(fp, format, ...);

// POSIX
int fd = open(path.c_str(), O_WRONLY | O_CREAT);
write(fd, buf, size);

// C++ 流
std::ofstream ofs(path);
ofs << data;

// 框架工具
SaveToFile(filePath, data);       // 常见封装
bool WriteStringToFile(path, str); // 文本写入
```

### 搜索模式

```bash
rg -n "fopen|fwrite|fprintf|ofstream|open\s*\(\s*\w|write\s*\(\s*\w+,|SaveToFile|WriteToFile|WriteStringToFile|FileWriter" --type cpp
```

## 3. JSON / 配置序列化

### 常见 API

```cpp
// nlohmann/json
nlohmann::json j;
j[key] = value;
std::ofstream o(path);
o << j.dump(4);

// cJSON
cJSON* root = cJSON_CreateObject();
cJSON_AddStringToObject(root, "key", value);
char* str = cJSON_Print(root);
// ... write str to file

// Parcel 序列化
Parcel parcel;
parcel.WriteString(value);
parcel.WriteInt32(num);
// ... parcel data written to persistent storage
```

### 搜索模式

```bash
rg -n "\.dump\s*\(|cJSON_Add|json\[|ToJson|Serialize|WriteParcelable|Parcel::Write" --type cpp
```

## 4. 偏好设置 / SharedPreferences

### 常见 API

```cpp
preferences_->PutString(key, value);
preferences_->PutInt(key, value);
preferences_->PutBool(key, value);
preferences_->Flush();
preferences_->FlushSync();
```

### 搜索模式

```bash
rg -n "PutString|PutInt|PutBool|preferences_->.*\(|Flush\(\)|FlushSync\(\)" --type cpp
```

## 5. 日志写入

### 常见 API

```cpp
TAG_LOGI(LABEL, "format %{public}s", value);
TAG_LOGD(LABEL, "format %{public}d", value);
TAG_LOGW(LABEL, "format %{public}s", value);
TAG_LOGE(LABEL, "format %{public}s", value);
HILOG_INFO(LABEL, "format %{public}s", value);
HILOG_WARN(LABEL, "format");
HILOG_ERROR(LABEL, "format");
```

### 搜索模式

```bash
rg -n "TAG_LOG|HILOG_INFO|HILOG_WARN|HILOG_ERROR|HILOG_DEBUG" --type cpp
```

### 日志注入检查要点

检查日志 format 字符串中是否包含 `%{public}s` 且参数来自外部输入。
外部输入的 `\r\n` 字符可能污染日志解析。

## 6. DataShare / DataAbility 持久化

DataShare 是 OpenHarmony 的数据持久化入口，跨应用数据访问的主要路径：

```cpp
// DataShare 写入接口
int Insert(const Uri &uri, const DataShareValuesBucket &value);
int Update(const Uri &uri, const DataSharePredicates &predicates, const DataShareValuesBucket &value);
int Delete(const Uri &uri, const DataSharePredicates &predicates);
std::shared_ptr<DataShareResultSet> Query(const Uri &uri, const DataSharePredicates &predicates, ...);
```

### 搜索模式

```bash
rg -n "DataShareValuesBucket|DataSharePredicates|DataShareResultSet|Insert\s*\(\s*Uri|Update\s*\(\s*Uri" --type cpp
```

## 7. IPC/Parcel 反序列化（间接持久化）

IPC 反序列化后的数据如果未经校验就写入持久化存储，形成高危链路：

```cpp
// 常见反序列化入口
std::string value = parcel.ReadString();
int32_t num = parcel.ReadInt32();
bool flag = parcel.ReadBool();
std::shared_ptr<SomeType> obj = parcel.ReadParcelable<SomeType>();

// Want 参数解析
std::string param = want.GetStringParam(key);
int param = want.GetIntParam(key, defaultVal);
```

### 搜索模式

```bash
rg -n "parcel\.Read|Parcel::Read|ReadParcelable|ReadString|ReadInt32|GetStringParam|GetIntParam|GetParam" --type cpp
```

## 8. 外部输入源识别（Source Patterns）

在反向污点追踪中，标记以下入口为外部输入源：

| 入口类型 | 典型代码位置 | 可信度 |
|---------|------------|-------|
| **services/*/src/*stub.cpp** | IPC Stub 自动生成代码 | 高（所有 IPC 入口） |
| **services/*/src/*_handler.cpp** | IPC 消息处理器 | 高 |
| **frameworks/js/napi/** | NAPI 绑定层 `GetParam()` | 高（JS 侧入参） |
| **frameworks/native/ability/native/** | Ability/Extension 生命周期回调参数 | 中 |
| **tools/aa/src/** | CLI 工具参数解析 | 中 |
