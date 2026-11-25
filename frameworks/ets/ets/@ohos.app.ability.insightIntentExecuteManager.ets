/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';

export default namespace insightIntentExecuteManager {

  loadLibraryWithPermissionCheck("insight_intent_execute_manager_ani_kit.z", "@ohos.app.ability.insightIntentExecuteManager")

  export native function nativeGetInsightIntentCallerBundleName(instanceId: int, callback: AsyncCallbackWrapper<string>): void;

  export function getInsightIntentCallerBundleName(instanceId: int): Promise<string> {
    return new Promise<string>((resolve: (data: string) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<string>((err: BusinessError | null, data: string | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as string);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        insightIntentExecuteManager.nativeGetInsightIntentCallerBundleName(instanceId, asyncCall);
      });
    });
  }
}