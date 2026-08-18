/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

class AbilityStage {
  constructor() { }
  onCreate() { }
  onDestroy() { }
  onAcceptWant(want) {
    hilog.sLogI(domainID, TAG, 'onAcceptWant, want:' + (want ? want.abilityName : 'undefined'));
  }
  onNewProcessRequest(want) {
    hilog.sLogI(domainID, TAG, 'onNewProcessRequest, want:' + (want ? want.abilityName : 'undefined'));
  }
  onConfigurationUpdated(config) { }
  onConfigurationUpdate(newConfig) { }
  onMemoryLevel(level) { }
}

export default AbilityStage;
