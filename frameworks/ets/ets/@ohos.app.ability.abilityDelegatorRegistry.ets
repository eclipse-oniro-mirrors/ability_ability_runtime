/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

import { AbilityDelegator as _AbilityDelegator } from 'application.AbilityDelegator';
import { AbilityDelegatorArgs as _AbilityDelegatorArgs } from 'application.abilityDelegatorArgs';
import { AbilityMonitor as _AbilityMonitor } from 'application.AbilityMonitor';
import { AbilityStageMonitor as _AbilityStageMonitor } from 'application.AbilityStageMonitor';
import { ShellCmdResult as _ShellCmdResult } from 'application.shellCmdResult';
import { InteropAbilityMonitor as _InteropAbilityMonitor} from 'application.InteropAbilityMonitor';

export default namespace abilityDelegatorRegistry {
  loadLibraryWithPermissionCheck("ability_delegator_registry_ani_kit.z", "@ohos.app.ability.abilityDelegatorRegistry")

  export native function getAbilityDelegator(): AbilityDelegator;

  export native function getArguments(): AbilityDelegatorArgs;

  export enum AbilityLifecycleState {
    UNINITIALIZED,
    CREATE,
    FOREGROUND,
    BACKGROUND,
    DESTROY
  }
  export type AbilityDelegator = _AbilityDelegator;
  export type AbilityDelegatorArgs = _AbilityDelegatorArgs;
  export type AbilityMonitor = _AbilityMonitor;
  export type ShellCmdResult = _ShellCmdResult;
  export type AbilityStageMonitor = _AbilityStageMonitor;
  export type InteropAbilityMonitor = _InteropAbilityMonitor;
}
