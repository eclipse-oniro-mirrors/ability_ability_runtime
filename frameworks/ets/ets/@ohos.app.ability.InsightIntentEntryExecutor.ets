/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
import insightIntent from '@ohos.app.ability.insightIntent';
import { IntentResultWrapper } from './utils/AbilityUtils';
import InsightIntentContext from '@ohos.app.ability.InsightIntentContext';
import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import { RecordData } from '@ohos.base';

export default class InsightIntentEntryExecutor<T> {
  context: InsightIntentContext = {};
  executeMode: insightIntent.ExecuteMode = insightIntent.ExecuteMode.UI_ABILITY_FOREGROUND;
  windowStage?: window.WindowStage;
  uiExtensionSession?: UIExtensionContentSession;
  public nativeExecutor: long = 0;

  public native nativeOnExecuteResult(data: Record<string, RecordData>): void;
  public native nativeOnExecuteError(err: Error): void;

  private checkPromise(obj: Any): boolean {
    return obj instanceof Promise;
  }

  public callPromise(p: Promise<insightIntent.IntentResult<T>>): void {
    p.then((result: insightIntent.IntentResult<T>): void => {
      let recordResult: Record<string, RecordData> = IntentResultWrapper.toRecord(result);
      this.nativeOnExecuteResult(recordResult);
    }).catch((err: Error): void => {
      this.nativeOnExecuteError(err);
    });
  }

  onExecute(): Promise<insightIntent.IntentResult<T>> {
    let ret: insightIntent.IntentResult<T> = { code: 0 };
    return Promise.resolve(ret);
  }
}
