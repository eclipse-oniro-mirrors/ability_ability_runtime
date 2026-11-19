# ohos-aa

## 概述

ohos-aa 是 OpenHarmony 提供的 Ability 管理命令行工具，用于在系统上启动指定的Ability组件或强制终止应用程序。该工具遵循 Claw 规范，以 JSON 格式输出执行结果，并提供详细的错误码、错误原因和解决建议，ohos-aa的安装路径为 `/system/bin/cli_tool/executable/ohos-aa`。

### 目录结构

```
tools/ohos-aa/
├── BUILD.gn                              # GN 构建配置
├── ohos-aa.json                          # Claw CLI 命令规范定义文件
├── include/
│   └── ohos_aa_command.h                 # 主头文件，定义命令选项、帮助信息和错误码
├── src/
│   ├── main.cpp                          # 入口函数，包含命令超时管理
│   └── ohos_aa_command.cpp               # 核心命令实现（start、force-stop、help）
└── tests/
    ├── BUILD.gn                          # 测试构建配置
    ├── ohos_aa_command_start_test.cpp    # start 子命令单元测试
    ├── ohos_aa_command_force_stop_test.cpp # force-stop 子命令单元测试
    └── ohos_aa_command_util_test.cpp     # 工具函数单元测试
```

## CLI 子命令表

| 子命令 | 作用 | 可选参数 | 所需权限 |
|--------|------|----------|----------|
| `start` | 启动一个 Ability 组件 | `--abilityname`、`--bundlename`、`--modulename`、`--uri`、`--action`、`--entity`、`--type`、`--time`、`--pi`、`--ps`、`--pb`、`--psn`、`--sandboxCloneIndex`、`--creatorBundle`、`--help` | `ohos.permission.cli.START_ABILITY` |
| `force-stop` | 强制停止指定应用及其进程 | `--bundlename`、`--help` | `ohos.permission.cli.KILL_APP_PROCESSES` |
| `--help` / `help` | 显示帮助信息 | 无 | 无 |

### start 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `--abilityname <name>` | string | 待启动的 Ability 名称 |
| `--bundlename <name>` | string | 待启动应用所在的 Bundle 名称 |
| `--modulename <name>` | string | 待启动的模块名称（多 HAP 应用中使用） |
| `--uri <uri>` | string | 用于隐式启动的 URI |
| `--action <action>` | string | 用于隐式启动的 Action |
| `--entity <entity>` | string | 用于隐式启动的 Entity |
| `--type <type>` | string | 用于隐式启动的 MIME 类型 |
| `--time` | flag | 等待 Ability 启动完成并测量启动耗时 |
| `--pi <json>` | string | 整型参数键值对，JSON 格式，如 `'{"key1":100,"key2":101}'` |
| `--ps <json>` | string | 字符串参数键值对，JSON 格式，如 `'{"key1":"value1","key2":"value2"}'` |
| `--pb <json>` | string | 布尔参数键值对，JSON 格式，如 `'{"key1":true,"key2":false}'` |
| `--psn <type>` | string | 空键对应的字符串类型值 |
| `--sandboxCloneIndex <index>` | integer  | 待启动的沙箱分身应用的索引（取值范围：2000-3000）|
| `--creatorBundle <name>` | string | 沙箱分身应用的创建方包名 |
| `--help` | flag | 显示 start 子命令帮助信息 |

> **注意**：显式启动时 `--abilityname` 和 `--bundlename` 必须同时提供。仅提供 `--abilityname` 而未提供 `--bundlename` 将导致错误。

### force-stop 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `--bundlename <name>` | string | 待停止应用的 Bundle 名称 |
| `--help` | flag | 显示 force-stop 子命令帮助信息 |

## Claw 规范遵循情况

### 命令命名规范

- 工具名称采用 `ohos-<domain>` 格式：`ohos-aa`
- 子命令使用小写英文，多词子命令以连字符分隔：`start`、`force-stop`
- 参数采用双连字符前缀的驼峰命名：`--abilityname`、`--bundlename`
- 命令规范元数据通过 JSON 配置文件 `ohos-aa.json` 定义

### 输入格式规范

- 命令行参数使用 `getopt_long` 进行解析，支持长选项格式
- 复杂参数（`--pi`、`--ps`、`--pb`）使用 JSON 字符串格式传入，需用单引号包裹以避免 Shell 转义
- 输入参数定义在 `ohos-aa.json` 的 `inputSchema` 字段中，采用 JSON Schema 规范

### 输出格式规范

所有命令执行结果均以 JSON 格式输出到标准输出，符合 `ohos-aa.json` 中 `outputSchema` 的定义。

**成功响应：**

```json
{
  "type": "result",
  "status": "success",
  "data": {
    "message": "start ability successfully."
  }
}
```

**失败响应：**

```json
{
  "type": "result",
  "status": "failed",
  "errCode": "ERR_ABILITY_NOT_FOUND",
  "errMsg": "The specified ability does not exist. The specified Ability is not installed.",
  "suggestion": "1. Check if the parameter abilityName of ohos-aa -a and the parameter bundleName of -b are correct\n2. Check if the application corresponding to the specified bundleName is installed\n3. For multi-HAP applications, it is necessary to confirm whether the HAP to which the ability belongs has been installed"
}
```

**带计时信息的成功响应（使用 `--time`）：**

```json
{
  "type": "result",
  "status": "success",
  "data": {
    "message": "StartMode: Cold\nBundleName: com.example.app\nAbilityName: EntryAbility\nTotalTime: 1200\nWaitTime: 1500"
  }
}
```

### 错误码

ohos-aa 定义了以下错误码，在命令执行失败时通过 JSON 输出返回：

| 错误码 | 说明 |
|--------|------|
| `ERR_INVALID_COMMAND` | 无效命令 |
| `ERR_INVALID_INPUT` | 无效的输入参数 |
| `ERR_ABILITY_VISIBLE_FALSE_DENY_REQUEST` | 目标 Ability 可见性校验失败 |
| `ERR_ABILITY_NOT_FOUND` | 指定的 Ability 不存在 |
| `ERR_ABILITY_SERVICE_NOT_CONNECTED` | Ability 服务连接失败 |
| `ERR_GET_ABILITY_SERVICE_FAILED` | 获取 Ability 服务失败 |
| `ERR_APP_RESOLVE_APP_ERR` | BMS 返回的应用信息异常 |
| `ERR_ABILITY_NO_FOUND_ABILITY_BY_CALLER` | 不支持通过 ohos-aa 启动 UIExtensionAbility |
| `ERR_ABILITY_IMPLICIT_START_ABILITY_FAIL` | 隐式启动未找到匹配应用 |
| `ERR_APP_CLONE_INDEX_INVALID` | appCloneIndex 参数无效 |
| `ERR_ABILITY_START_ABILITY_WAITING` | 有其他 Ability 正在启动中 |
| `ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE` | 开发者模式下解锁屏幕失败 |
| `ERR_CROWDTEST_EXPIRED` | 众测应用已过期 |
| `ERR_APP_CONTROLLED` | 目标应用被管控 |
| `ERR_EDM_APP_CONTROLLED` | 目标应用被企业设备管理管控 |
| `ERR_NOT_SUPPORTED_PRODUCT_TYPE` | 当前设备不支持窗口选项 |
| `ERR_STATIC_CFG_PERMISSION` | 指定进程权限校验失败 |
| `ERR_INNER_ERR_START` | 内部错误（内存不足、超时等） |
| `ERR_GET_BUNDLE_INFO_FAILED` | 获取包信息失败 |
| `ERR_KILL_PROCESS_FAILED` | 杀进程失败 |
| `ERR_KILL_PROCESS_KEEP_ALIVE` | 常驻进程无法终止 |

## 使用示例

### 查看帮助信息

```bash
# 查看 ohos-aa 总体帮助
ohos-aa --help

# 查看 start 子命令帮助
ohos-aa start --help

# 查看 force-stop 子命令帮助
ohos-aa force-stop --help
```

### 启动 Ability（显式启动）

```bash
# 基本显式启动
ohos-aa start --abilityname EntryAbility --bundlename com.example.app
```

### 启动 Ability（带模块名）

```bash
# 指定模块名启动（适用于多 HAP 应用）
ohos-aa start --abilityname EntryAbility --bundlename com.example.app --modulename entry
```

### 启动 Ability（隐式启动）

```bash
# 通过 Action 和 Type 隐式启动
ohos-aa start --action ohos.want.action.view --type text/plain --uri "https://www.example.com/page"
```

### 启动 Ability（带参数传递）

```bash
# 传递整型、字符串、布尔参数
ohos-aa start --abilityname EntryAbility --bundlename com.example.app \
  --pi '{"pageId":1,"count":100}' \
  --ps '{"theme":"dark","language":"zh"}' \
  --pb '{"debug":true,"fullscreen":false}'
```

### 启动 Ability（带启动耗时测量）

```bash
# 使用 --time 选项测量启动耗时
ohos-aa start --abilityname EntryAbility --bundlename com.example.app --time
```

### 强制停止应用

```bash
# 强制停止指定应用
ohos-aa force-stop --bundlename com.example.app
```
