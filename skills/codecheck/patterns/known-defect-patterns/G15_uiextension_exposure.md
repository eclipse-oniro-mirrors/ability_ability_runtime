# G15 UIExtension / 窗口组件暴露面

> 来源：known-defect-patterns.md G15。

## 特征信号 / grep 线索

- `UIExtensionComponent` / `WindowExtension` 等组件可被任意三方应用加载，未校验加载者身份。
- 窗口弹框（如权限弹框、跳转弹框）的显示内容/跳转目标由外部传入参数控制，未做一致性校验。
- `UIExtensionAbility` 可被循环引用保活（如 A 加载 B、B 加载 A）。
- 悬浮窗场景下对弹框内容未做防护，可被覆盖/篡改。

## 历史案例

- `UIExtensionComponent` 无法保证界面输入源自用户输入 → 任意三方应用可加载系统 UIExtension 组件并模拟点击，后端 120+ 应用组件存在风险。
- `amsdialog` 通知弹框未针对悬浮窗场景做防护 → 任意应用可通过悬浮窗篡改跳转申请弹窗内容和选项，实现跳转应用混淆。
- `JumpInterceptorDialog` 组件暴露，跳转组件名称和界面显示名称由外部传入参数控制 → 恶意应用传入正常跳转目标参数，实际打开截屏/录屏组件。
- 利用循环引用的 `UIExtensionAbility` 可保活进程，实现无法被杀死的恶意应用。
- `WindowExtension` 文档声明仅系统应用可用，但实际三方应用也可使用。

## 检查点

- `UIExtension` / `WindowExtension` 加载是否校验调用者身份（非任意三方可加载系统组件）？
- 弹框/跳转的目标和内容是否由服务端决定，而非完全信任外部传入参数？
- 窗口层级是否有防覆盖机制（悬浮窗不能覆盖系统弹框）？
- 循环引用检测：同一进程链中 UIExtension 嵌套深度是否有上限？
