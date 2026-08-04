# IPC 序列化模式 (IPC Serialization)

> 所有 scanner 共享的 IPC 序列化/反序列化缺陷模式词典。

---

## IPC-001 ReadFromParcel / WriteToParcel 字段顺序不匹配

**信号特征**
- 序列化端与反序列化端字段顺序不一致 → 读错类型。
- `Parcel` 被重置位置后重新解析，校验与执行不一致。

**grep 线索**
```bash
rg -n "ReadFromParcel|WriteToParcel" --type cpp
rg -n "ReadString|ReadInt32|ReadBool|ReadParcelable" --type cpp
rg -n "WriteString|WriteInt32|WriteBool|WriteParcelable" --type cpp
```

**典型后果**：类型混淆、内存溢出、参数走私，P0。

---

## IPC-002 枚举值直接 static_cast 无范围校验

**信号特征**
- 从 Parcel 读出的枚举值直接 `static_cast` 使用，未校验是否在合法范围。
- 恶意 IPC 可注入非法枚举值 → 越界访问、逻辑错乱。

**grep 线索**
```bash
rg -n "static_cast<.*>" --type cpp   # 结合 ReadInt32
rg -n "parcel.ReadInt32" --type cpp
```

**典型后果**：越界、逻辑绕过，P0/P1。

---

## IPC-003 size 字段未校验负数

**信号特征**
- `int32_t size` 从 Parcel 读取后未校验 `size < 0` 即用于 `resize`/`new`/循环。
- 负数转 unsigned 变巨大值 → OOM。

**grep 线索**
```bash
rg -n "ReadInt32" --type cpp
rg -n "resize\(|new\s+\w+\[" --type cpp
```

**典型后果**：OOM、越界，P0。

**历史案例**：G3（`InsightIntentExecuteResult::ReadFromParcel` int32 直传 `resize` → OOM）。

---

## IPC-004 fd 跨 IPC 边界所有权

**信号特征**
- fd 通过 Parcel 传递时未 `dup`，接收方 close 影响发送方。
- fd 在异常路径未 close。

**grep 线索**
```bash
rg -n "WriteFileDescriptor|ReadFileDescriptor" --type cpp
rg -n "dup\s*\(" --type cpp
```

**典型后果**：fd 泄漏/误关，P1。

---

## IPC-005 校验位置错（位置错）

**信号特征**
- 鉴权做在客户端（Proxy 层），服务端裸奔 → 攻击者可直接 `GetSystemAbility` 拼 Parcel 调 code 绕过。
- 多次 IPC 只覆盖第一跳（入口 code 有鉴权，内部 code 无鉴权）。
- 同一服务部分 code 漏挂鉴权（调试/测试/遗留 code 未走鉴权分支）。

**grep 线索**
```bash
rg -n "OnRemoteRequest|OnRemoteRequestEx" --type cpp
rg -n "CheckPermission|VerifyAccessToken|PermissionVerification" --type cpp
rg -n "case\s+.*:" --type cpp   # 检查每个 code 分支
```

**典型后果**：权限绕过，P0。

**历史案例**：G2（`CliToolManagerService::ExecTool` 未鉴权路径、`StartUIExtensionAbility` 未校验调用者）。

---

## IPC-006 身份来源错（身份错）

**信号特征**
- 从 `data`（Parcel）读取 `uid`/`bundleName`/`token` 做判断，而非 `IPCSkeleton::GetCallingUid()`/`GetCallingTokenID()`。
- 直接读取客户端传的 `isSystemApp`/`permissionGranted`/`callerType` 布尔值。
- 对 `TOKEN_NATIVE`/`ROOT_UID`/`SHELL_UID` 直接放行。
- 信任 `getpid`/`GetCallingPid` 作为身份依据（pid 回绕）。

**grep 线索**
```bash
rg -n "GetCallingPid|getpid" --type cpp
rg -n "GetCallingUid|GetCallingTokenID" --type cpp
rg -n "data.ReadString|data.ReadInt32" --type cpp   # 是否读身份
rg -n "TOKEN_NATIVE|ROOT_UID|SHELL_UID" --type cpp
```

**典型后果**：身份伪造、任意操作，P0。

**历史案例**：G1（`UninstallAppInner` 用 pid 校验 → 卸载任意应用、`specifiedFullTokenId` 外部可控 → token 伪造）。

---

## IPC-007 调用链中转错（链路错）

**信号特征**
- SA 中继：只校验直接调用方 token，未区分 `isProxy`、未对原始调用方二次授权。
- 服务端用 `ResetCallingIdentity()`/`clearCallingIdentity()` 后未恢复身份就鉴权 → 替调用方提权。

**grep 线索**
```bash
rg -n "ResetCallingIdentity|clearCallingIdentity" --type cpp
rg -n "isProxy" --type cpp
```

**典型后果**：中继提权，P0。

---

## IPC-008 分发与解析错（分发错）

**信号特征**
- `switch` 的 `default` 分支有副作用（未知 code 静默执行公共逻辑）。
- 校验失败只打日志未 return（或错误码比较写反 `==` vs `!=`，用 0 判断成功而接口返回负值）。
- 校验时读取的字段与执行时反序列化的对象不一致（反序列化绕过）。
- 先产生副作用（`WriteFile`/`StartAbility`/发广播）再鉴权。
- `try-catch` 中校验异常被捕获后直接 fallthrough 执行后续逻辑。

**grep 线索**
```bash
rg -n "default:" --type cpp
rg -n "return\s+true|return\s+ERR_OK" --type cpp   # 错误分支返回成功
rg -n "try\s*\{|catch\s*\(" --type cpp
```

**典型后果**：未授权操作、绕过校验，P0。

**历史案例**：G14（异常分支打印日志后返回 `true`/`ERR_OK`）、高影响清单 4.4/7.3/7.4。

---

## IPC-009 校验逻辑缺陷（逻辑错）

**信号特征**
- 黑名单式校验（大小写、`/../`、符号链接、等价 Unicode 可绕过）。
- 前缀/包含匹配代替精确匹配（`find()`/`starts_with()` 做包名校验，`com.ohos.evil` 可通过）。
- 只校验"能不能调"，不校验"对谁做"（权限通过后允许操作 Parcel 任意目标）。
- 参数组合校验不全（只校验 `action` 未校验 `uri`/`extras`）。
- 整数截断/溢出绕过范围检查（`int32` 读 `uint32` 溢出后绕过 `index < MAX`）。

**grep 线索**
```bash
rg -n "find\(|starts_with|substr" --type cpp
rg -n "static_cast<int>" --type cpp
```

**典型后果**：绕过校验、越权，P0。

---

## IPC-010 跨设备分布式身份采信（状态错）

**信号特征**
- 本端直接采信对端设备传来的 `userId`/包名/权限结论，未基于 `IPCSkeleton` 真实调用方重新鉴权。
- 对端设备本身未完成可信认证。
- 转发链路上身份可被篡改。

**grep 线索**
```bash
rg -n "distributed|DistributedClient" --type cpp
rg -n "ContinueMission|distributedMission" --type cpp
```

**典型后果**：跨设备身份伪造，P0/P1。

**历史案例**：ipc-auth-checklist 6.2。
