/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import Context from 'application.Context'
import { Configuration } from '@ohos.app.ability.Configuration'
import { ElementName } from 'bundleManager.ElementName'
import { HapModuleInfo } from 'bundleManager.HapModuleInfo'

export default class AbilityStageContext extends Context {
  static {
    loadLibraryWithPermissionCheck("ability_stage_context_ani_kit.z", "application.AbilityStageContext");
  }
  config?: Configuration;
  currentHapModuleInfo!: HapModuleInfo;
  launchElement?: ElementName;

  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'AbilityStageContext';

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    console.log('transferStatic type: ' + type);
    return AbilityStageContext.nativeTransferStatic(ESValue.wrap(input), type);
  }

  static transferDynamic(input: Object): Any {
    return AbilityStageContext.nativeTransferDynamic(input).unwrap();
  }
}
