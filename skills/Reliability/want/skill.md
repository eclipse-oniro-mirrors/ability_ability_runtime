---
name: want-reliability
description: Want可靠性skill。在Want基础库代码开发或设计阶段，自动提醒Want序列化/反序列化、fd传递、自定义参数注入等可靠性关注点。触发场景：want代码开发、want_params代码修改、序列化反序列化开发、fd传递代码修改、want_params_wrapper修改、array_wrapper修改、pac_map修改。当用户开发或修改Want基础库相关代码时，此skill基于故障模式库和资源限制规格主动提示可靠性风险。
allowed-tools: [Read, Glob, Grep, Bash, Write, Task, question]
metadata:
  tags: [reliability, want, serialization, fd, fault-mode, resource-limit]
---

# Want可靠性skill

## 使用场景

当用户修改Want基础库相关代码或使用want时使用此skill：

- Want/WantParams序列化/反序列化代码
- fd传递相关代码
- 自定义参数处理代码
- Array/PacMap序列化代码

## 代码路径

| 代码路径 | 说明 |
| --- | --- |
| ability_base仓 `interfaces/kits/native/want/src/`（want.cpp、want_params.cpp、array_wrapper.cpp、pac_map.cpp 等） | Want核心实现 |
| ability_base仓 `interfaces/kits/native/want/include/` | Want头文件 |

## 基础库健壮性原则

Want是基础库，被大量系统服务和应用调用，**外部输入不能导致Want模块出现crash或死循环**。这是Want基础库的底线要求，所有代码修改必须遵守：

- **不可crash**：外部传入的任何数据（WantParams、Array、PacMap、序列化数据等）不可导致Want模块空指针、越界访问、除零等crash。所有外部输入必须做合法性校验后再使用。
- **不可死循环**：外部传入的数据不可触发Want模块内的死循环（如递归解析无深度限制、循环引用未检测等）。所有递归和循环逻辑必须有明确的终止条件。
- **异常隔离**：Want模块处理外部数据出错时，应返回错误码而非崩溃，确保调用方能够降级处理。

## 故障模式（3条）

### FM-Want-01: 序列化/反序列化不匹配（I级 - 应用拉起失败）

| 字段 | 内容 |
|---|---|
| 故障模式 | I-04 序列化/反序列化不匹配 |
| 描述 | proxy端与stub端序列化格式不匹配 |
| 严酷度 | I |
| 引发原因 | proxy端和stub端版本不一致（broker场景、分布式跨设备场景等） |
| 影响 | 应用拉起失败 |
| 关注代码 | want.cpp Marshalling/UnMarshalling, want_params.cpp |

**开发关注点**：
- 修改序列化逻辑时必须保持向后兼容，proxy端和stub端可能运行不同版本
- broker场景：设备升级过程中新旧版本并存，proxy/stub版本可能不一致
- 分布式场景：跨设备调用时两端系统版本可能不同，序列化格式必须兼容
- 新增字段需有默认值，旧版本读取新数据时需能跳过未知字段
- 序列化/反序列化需版本协商机制

### FM-Want-02: fd/mmap资源泄漏（II级 - 资源泄露）

| 字段 | 内容 |
|---|---|
| 故障模式 | F-06 fd/mmap资源泄漏 |
| 描述 | FD资源泄露 |
| 严酷度 | II |
| 引发原因 | Want fd传递路径fd未正确关闭 |
| 影响 | fd泄漏导致资源耗尽 |
| 关注代码 | want.cpp fd传递 |

**开发关注点**：
- fd传递的所有路径（正常+异常）都必须关闭fd
- IPC调用失败后需回收已传递的fd
- fd的生命周期必须明确（谁负责关闭）

### FM-Want-03: 自定义参数滥用（I级 - 权限拦截失效）

| 字段 | 内容 |
|---|---|
| 故障模式 | 数据注入 |
| 描述 | want自定义参数滥用 |
| 严酷度 | I |
| 引发原因 | WantParams自定义参数未做校验，系统key被外部注入 |
| 影响 | 权限拦截失效，恶意应用通过Want参数绕过权限检查或伪造系统行为 |
| 关注代码 | want_params.cpp, want参数校验逻辑, AMS中WantParams读写点 |

**开发关注点**：
- WantParams中的自定义参数需做大小和内容校验
- 权限相关参数不可通过自定义参数覆盖
- 系统应用读取Want参数时需校验来源合法性
- **系统key必须由程序框架域系统代码设置，不可接受外部传入的同名key**（详见下节）
- **尽量不使用WantParams传递系统属性**：WantParams本质是开放的key:value容器，外部可自由构造。在条件允许的情况下，优先使用其他可序列化的结构体来承载和传递新增的系统属性，而不是放在WantParams中。结构体字段有明确的类型约束和访问控制，能从根源上避免外部注入系统属性的风险

## WantParams系统Key保护

### 背景机制

WantParams的设计意图是给调用方传递自定义 key:value 数据。但程序框架域有一些特殊的 key 也放在 WantParams 中传递，这些 key **只能由程序框架域系统代码设置**，不能由外部（尤其三方开发者）指定。

### 系统Key的风险

当三方开发者构造的Want中携带了这些系统key时，可能导致：

1. **权限绕过**：三方应用伪造系统key跳过权限检查（如伪造成系统应用行为）
2. **行为劫持**：三方应用通过系统key改变Ability启动行为（如指定窗口模式、启动场景等）
3. **数据注入**：三方应用注入系统key影响下游系统服务的决策逻辑

### 系统开发者检查要求

**在程序框架域代码中读写WantParams的key时，必须检查：**

1. **设置系统key时**：确认调用来源是系统代码（SA/系统应用），不是三方应用
   - 检查调用方UID/PID是否为系统进程
   - 检查调用方是否有对应系统权限
   - 三方应用传入的系统key必须在入口处被过滤或拒绝

2. **读取系统key时**：确认该key是由系统代码设置的，不是外部传入的
   - 不应无条件信任WantParams中的系统key值
   - 对关键的系统key做来源校验（谁设置的）

3. **新增系统key时**：
   - 评估该key是否可以被三方应用构造
   - 如果可以被三方应用构造，必须在AMS入口处增加过滤/校验逻辑
   - 在需求设计文档中明确标注该key为系统key，不可由外部指定

### 常见系统Key场景（示例）

| 场景 | 说明 | 风险 |
|---|---|---|
| 启动场景标识 | 标识Ability启动来源/场景的系统参数 | 三方伪造可改变启动行为 |
| 窗口/显示模式 | 控制Ability窗口模式的系统参数 | 三方伪造可绕过窗口模式限制 |
| 权限/身份相关 | 与权限校验或身份识别相关的参数 | 三方伪造可绕过权限检查 |
| 系统服务路由 | 控制系统服务路由的参数 | 三方伪造可劫持服务路由 |

> **注意**：以上为场景示例，具体系统key清单需根据代码中实际使用情况维护。开发新增系统key时请同步更新此清单。

### 代码检查方法

在AMS/Ability管理代码中检索WantParams的key读写点：

```
Grep检索关键词：
- "SetParam" / "GetParam" / "GetParams" / "HasParam"
- WantParams相关key常量定义
- 检查每个key的设置路径是否限制了调用方来源
```

## 资源限制规格（14条：ability_base 11条 + ability_runtime 3条）

### Want IPC序列化限制（ability_base，11条）

| 限制项 | 规格值 | 超限影响 |
|---|---|---|
| WantParams递归深度 | 100层 | Marshalling/UnMarshalling返回false |
| WantParams不支持数据大小 | 100MB | UnsupportedData写入/读取失败 |
| WantParams数组/键值对数量 | 1024 | 反序列化返回false |
| Want Parcel容量 | 512KB | Want IPC写入失败 |
| Array数组元素数 | 50MB | 构造时静默截断（数据丢失）；反序列化返回false |
| Array数组递归深度 | 100层 | Parse/ToString返回nullptr/空串 |
| Array声明大小校验 | size > 实际长度+1 | 视为DoS攻击，返回nullptr |
| WantParamWrapper解析递归深度 | 100层 | ToString/Parse失败 |
| WantParams JSON序列化深度 | 100层 | JSON转换返回false |
| PacMap递归深度 | 100层 | ToJson/ParseJson返回false |
| PacMap数组元素数 | 1024 | JSON解析返回false |

### Want IPC容量限制（ability_runtime，3条）

| 限制项 | 规格值 | 超限影响 |
|---|---|---|
| Want IPC容量 | 216KB | Want IPC写入失败，Ability启动/连接失败 |
| Want列表IPC容量 | 864KB | 批量启动Ability的IPC写入失败 |
| Want列表大小 | 4个 | 超过后批量启动被拒 |

**开发关注点**：
- 所有递归序列化必须有深度限制（当前100层）
- Array构造时静默截断是数据丢失风险，需打日志告警
- Want IPC容量216KB是硬限制，传递大数据需走其他通道（如fd/Ashmem）
- Array声明大小与实际长度校验是DoS防护，不可移除

## 开发检查清单

修改Want相关代码时，检查以下项：

- [ ] 序列化/反序列化是否保持向后兼容
- [ ] 新增字段是否有默认值
- [ ] fd传递的所有路径（正常+异常）是否都关闭fd
- [ ] IPC调用失败后是否回收已传递的fd
- [ ] WantParams自定义参数是否做了大小校验
- [ ] 递归序列化是否已有深度限制
- [ ] Array构造时是否对截断打了告警日志
- [ ] Want IPC数据是否在216KB限制内

### 基础库健壮性检查

- [ ] 外部输入是否做了合法性校验（非空、范围、类型）后才使用
- [ ] 外部输入是否可能导致空指针、越界访问、除零等crash
- [ ] 所有递归/循环逻辑是否有明确的终止条件，外部输入不可触发死循环
- [ ] 处理外部数据出错时是否返回错误码而非崩溃
- [ ] 异常路径是否完整释放已分配的资源

### WantParams系统Key保护检查

- [ ] **设置系统key时**：是否检查了调用方来源（UID/PID/权限），拒绝三方应用设置
- [ ] **读取系统key时**：是否校验了该key由系统代码设置，不无条件信任外部传入值
- [ ] **新增系统key时**：是否评估了该key能否被三方应用构造，能否在入口处过滤
- [ ] **新增系统key时**：是否在需求设计文档中标注为系统key，不可由外部指定
- [ ] **新增系统key时**：是否同步更新了系统key清单
- [ ] **AMS入口处**：是否对三方应用传入的系统key做了过滤或拒绝处理
