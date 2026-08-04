# 并发安全模式 (Concurrency Safety)

> 所有 scanner 共享的并发缺陷模式词典。

---

## CONC-001 死锁 / 锁顺序不一致

**信号特征**
- 两个函数以相反顺序获取两把锁（A→B vs B→A）。
- 持锁时调用外部代码（回调 / 虚函数 / 跨进程），外部可能获取同一锁。
- 同线程重入加锁（回调同步重入）→ 永久死锁。
- `join` / `wait` / `sleep` 在持锁路径上。

**grep 线索**
```bash
rg -n "lock_guard|unique_lock|scoped_lock" --type cpp
rg -n "mutex|rmutex|recursive_mutex" --type cpp
rg -n "join\(\)|wait\(|condition_variable" --type cpp
```

**锁三原则（历史高发）**
1. 禁止持锁回调外部代码。
2. 禁止同线程重入加锁（回调同步重入会永久死锁）。
3. 共享容器（观察者列表、回调 map）必须有明确持有者与锁保护。

**典型后果**：进程卡死，P0。

**历史案例**：高影响清单 6.1-6.4、锁三原则违反。

---

## CONC-002 竞态条件 (Check-Then-Act)

**信号特征**
- `if (instance_ == nullptr) { instance_ = new Instance(); }` 非原子 DCL。
- `if (cache.find(...))` + lock + 再次检查，非原子 double-checked locking。
- 标志位（`isOpen_` / 静态 bool）无锁读写用于状态判断。
- `int count_++;` 非原子自增。

**grep 线索**
```bash
rg -n "if\s*\(\s*instance_?\s*==\s*nullptr" --type cpp
rg -n "if\s*\(.*find\(.*\)\s*==" --type cpp   # check-then-act
rg -n "isOpen_|isInitialized" --type cpp
```

**触发条件**
- 多线程可能同时执行检查-使用路径。
- 共享变量无 `std::atomic` 或 mutex 保护。

**典型后果**：double-init、UAF、数据损坏，P0/P1。

**历史案例**：G12（`ExtractorUtil::GetExtractor` 非原子 DCL）、高影响清单 6.2。

---

## CONC-003 数据竞争 / 共享容器无保护

**信号特征**
- 回调容器（`mCancelCallbacks_`、`callProxyRecords_`、`observers_`）以 `vector`/`map`/`list` 存储，读写无 mutex/rwlock。
- `Handler`/`Proxy`/`sptr` 引用跨线程传递，注册/注销与触发/销毁不在同一线程。
- `death recipient` 回调与主业务逻辑并发修改同一对象状态。
- 全局 map/cache 读写未与 mutex 配对。
- `shared_ptr` 全局对象读写未加锁或未用 atomic。

**grep 线索**
```bash
rg -n "std::vector.*callback|std::map.*observer|std::list.*listener" --type cpp
rg -n "shared_ptr.*_\s*;" --type cpp   # 全局 shared_ptr 成员
rg -n "deathRecipient|DeathRecipient" --type cpp
rg -n "static\s+.*_\s*;" --type cpp   # 静态可变状态
```

**触发条件**
- 容器注册/注销/遍历不在同一线程或无显式锁。
- 对象销毁前未确保异步回调已注销/等待完成。

**典型后果**：UAF、记录丢失、内存破坏，P0/P1。

**历史案例**：G12（`pending_want_record.cpp` `mCancelCallbacks_`、`LocalCallContainer` `callProxyRecords_`、AMS `wmsHandler_` 竞争）、高影响清单 6.1/6.4。

---

## CONC-004 状态机竞争 / 后台管控绕过

**信号特征**
- 杀进程/清理后台逻辑依赖应用主动上报或仅发 `SIGTERM`，无 `SIGKILL` 兜底。
- 窗口状态与 Ability 状态非原子切换（窗口已隐藏但 Ability 仍在前台）。
- 拉起链路（`StartAbilityInner`/`ConnectAbility`）无递归深度计数/循环检测。
- `RestartApp`/`force-stop` 未校验应用真实终端状态。

**grep 线索**
```bash
rg -n "SIGTERM|kill\(|killpg" --type cpp
rg -n "APP_APPLICATION_TERMINATED|onApplicationTerminated" --type cpp
rg -n "StartAbilityInner|ConnectAbility" --type cpp
rg -n "RestartApp|force-stop|ForceStop" --type cpp
```

**触发条件**
- 恶意应用利用生命周期回调（`onPageHide`/`onBackground`）拉起自身或互相拉起形成死循环。
- 劫持 `APP_APPLICATION_TERMINATED` 上报接口 → 清空后台时免杀。

**典型后果**：进程杀不死、霸屏、保活，P0/P1。

**历史案例**：G11（毒王霸屏、RestartApp 状态校验不严、death 通知竞争保活）。
