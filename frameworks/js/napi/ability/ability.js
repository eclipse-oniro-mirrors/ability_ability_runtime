/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
let Callee = requireNapi('application.Callee');
let AbilityConstant = requireNapi('app.ability.AbilityConstant');
let hilog = requireNapi('hilog');

let domainID = 0xD001320;
let TAG = 'JSENV';

class Ability {
  constructor() {
    this.callee = new Callee('rpc.application.callee');
    hilog.sLogI(domainID, TAG, 'Ability::constructor callee is ' + typeof this.callee + ' ' + this.callee);
  }
  onCreate(want) { }
  onDestroy() { }
  onWindowStageCreate(windowStage) { }
  onWindowStageDestroy() { }
  onForeground(want) { }
  onBackground() { }
  onPrepareToTerminate() {
    return false;
  }
  onMemoryLevel(level) { }
  onWindowStageRestore(windowStage) { }
  onCallRequest() {
    hilog.sLogI(domainID, TAG, 'Ability::onCallRequest callee is ' + typeof this.callee + ' ' + this.callee);
    return this.callee;
  }
  onContinue(wantParams) { }
  onConfigurationUpdated(config) { }
  onConfigurationUpdate(newConfig) { }
  onNewWant(want, param) { }
  dump(params) { }
  onDump(params) { }

  onSaveState(state, wantParams) {
    return AbilityConstant.OnSaveResult.RECOVERY_AGREE;
  }
  onShare(wantParams) { }
  onCollaborate(wantParams) {
    return AbilityConstant.CollaborateResult.REJECT;
  }
}

export default Ability;