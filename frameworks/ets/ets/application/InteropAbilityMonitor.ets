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
import UIAbility from '@ohos.app.ability.UIAbility';

export default interface InteropAbilityMonitor {

  moduleName: string;
  abilityName: string;
  onAbilityCreate: (ability: UIAbility) => void;
  onAbilityForeground: (ability: UIAbility) => void;
  onAbilityBackground: (ability: UIAbility) => void;
  onAbilityDestroy: (ability: UIAbility) => void;
  onWindowStageCreate: (ability: UIAbility) => void;
  onWindowStageRestore: (ability: UIAbility) => void;
  onWindowStageDestroy: (ability: UIAbility) => void;

}

class InteropAbilityMonitorInner implements InteropAbilityMonitor {
  moduleName: string = "";
  abilityName: string = "";
  onAbilityCreate: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
  onAbilityForeground: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
  onAbilityBackground: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
  onAbilityDestroy: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
  onWindowStageCreate: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
  onWindowStageRestore: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
  onWindowStageDestroy: (ability: UIAbility) => void = (ability: UIAbility) => {
  };
}
