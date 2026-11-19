/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

let hilog = requireNapi('hilog');

let domainID = 0xD001320;
let TAG = 'JSENV';

class InsightIntentContext {
  constructor(obj) {
    this.__context_impl__ = obj;
  }

  startAbility(want, callback) {
    hilog.sLogI(domainID, TAG, 'startAbility');
    return this.__context_impl__.startAbility(want, callback);
  }

  setReturnModeForUIAbilityForeground(returnMode) {
    hilog.sLogI(domainID, TAG, 'setReturnModeForUIAbilityForeground');
    return this.__context_impl__.setReturnModeForUIAbilityForeground(returnMode);
  }

  setReturnModeForUIExtensionAbility(returnMode) {
    hilog.sLogI(domainID, TAG, 'setReturnModeForUIExtensionAbility');
    return this.__context_impl__.setReturnModeForUIExtensionAbility(returnMode);
  }

  get insightIntentExecutionInfo() {
    hilog.sLogI(domainID, TAG, 'insightIntentExecutionInfo');
    return this.__context_impl__.insightIntentExecutionInfo;
  }
}

export default InsightIntentContext;
