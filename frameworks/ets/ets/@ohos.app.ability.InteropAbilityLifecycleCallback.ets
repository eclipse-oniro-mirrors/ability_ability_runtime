/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import window from '@ohos.window';
import transfer from '@ohos.transfer';

type AbilityCallbackFn = (ability: Any) => void;
type WindowStageCallbackFn = (ability: Any, windowStage: window.WindowStage) => void;

interface InteropAbilityLifecycleCallback {
  onAbilityCreate: AbilityCallbackFn;
  onAbilityForeground: AbilityCallbackFn;
  onAbilityBackground: AbilityCallbackFn;
  onAbilityDestroy: AbilityCallbackFn;
  onWindowStageCreate: WindowStageCallbackFn;
  onWindowStageDestroy: WindowStageCallbackFn;
  onAbilityContinue?: AbilityCallbackFn;
  onWindowStageInactive?: WindowStageCallbackFn;
  onWindowStageActive?: WindowStageCallbackFn;
  onNewWant?: AbilityCallbackFn;
  onAbilityWillBackground?: AbilityCallbackFn;
  onAbilityWillForeground?: AbilityCallbackFn;
  onAbilityWillCreate?: AbilityCallbackFn;
  onWindowStageWillDestroy?: WindowStageCallbackFn;
  onAbilityWillDestroy?: AbilityCallbackFn;
  onWindowStageRestore?: WindowStageCallbackFn;
  onAbilitySaveState?: AbilityCallbackFn;
  onAbilityWillContinue?: AbilityCallbackFn;
  onAbilityWillSaveState?: AbilityCallbackFn;
  onWillNewWant?: AbilityCallbackFn;
  onWindowStageWillCreate?: WindowStageCallbackFn;
  onWindowStageWillRestore?: WindowStageCallbackFn;
}

namespace InteropAbilityLifecycle {
  function onAbilityCreateInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityCreate(ability.unwrap());
  };

  function onAbilityForegroundInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityForeground(ability.unwrap());
  };

  function onAbilityBackgroundInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityBackground(ability.unwrap());
  };

  function onAbilityDestroyInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityDestroy(ability.unwrap());
  };

  function onWindowStageCreateInner(ability: ESValue, windowStage: ESValue,
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageCreate(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  };

  function onWindowStageDestroyInner(ability: ESValue, windowStage: ESValue,
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageDestroy(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  };

  function onAbilityContinueInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityContinue?.(ability.unwrap());
  };

  function onWindowStageInactiveInner(ability: ESValue, windowStage: ESValue,
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageInactive?.(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  };

  function onWindowStageActiveInner(ability: ESValue, windowStage: ESValue,
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageActive?.(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  };

  function onNewWantInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onNewWant?.(ability.unwrap());
  };

  function onAbilityWillBackgroundInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityWillBackground?.(ability.unwrap());
  };

  function onAbilityWillForegroundInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityWillForeground?.(ability.unwrap());
  };

  function onAbilityWillCreateInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityWillCreate?.(ability.unwrap());
  };

  function onWindowStageWillDestroyInner(ability: ESValue, windowStage: ESValue, 
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageWillDestroy?.(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  };

  function onAbilityWillDestroyInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityWillDestroy?.(ability.unwrap());
  };

  function onWindowStageRestoreInner(ability: ESValue, windowStage: ESValue, 
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageRestore?.(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  };

  function onAbilitySaveStateInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilitySaveState?.(ability.unwrap());
  };

  function onAbilityWillContinueInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityWillContinue?.(ability.unwrap());
  };

  function onAbilityWillSaveStateInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onAbilityWillSaveState?.(ability.unwrap());
  }

  function onWillNewWantInner(ability: ESValue, callback: InteropAbilityLifecycleCallback): void {
    callback.onWillNewWant?.(ability.unwrap());
  };

  function onWindowStageWillCreateInner(ability: ESValue, windowStage: ESValue, 
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageWillCreate?.(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  }

  function onWindowStageWillRestoreInner(ability: ESValue, windowStage: ESValue, 
    callback: InteropAbilityLifecycleCallback): void {
    callback.onWindowStageWillRestore?.(ability.unwrap(),
      transfer.transferStatic(windowStage.unwrap(), 'window.WindowStage') as window.WindowStage);
  }
}

export default InteropAbilityLifecycleCallback;