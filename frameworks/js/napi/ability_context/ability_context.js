/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

let Context = requireNapi('application.Context');
let Caller = requireNapi('application.Caller');
let hilog = requireNapi('hilog');

let domainID = 0xD001320;
let TAG = 'JSENV';

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
class ParamError extends Error {
  constructor(msg) {
    super(msg);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class AbilityContext extends Context {
  constructor(obj) {
    super(obj);
    this.abilityInfo = obj.abilityInfo;
    this.currentHapModuleInfo = obj.currentHapModuleInfo;
    this.config = obj.config;
    this.windowStage = obj.windowStage;
  }

  onUpdateConfiguration(config) {
    this.config = config;
  }

  startAbility(want, options, callback) {
    return this.__context_impl__.startAbility(want, options, callback);
  }

  openLink(link, options, callback) {
    return this.__context_impl__.openLink(link, options, callback);
  }

  startAbilityAsCaller(want, options, callback) {
    return this.__context_impl__.startAbilityAsCaller(want, options, callback);
  }

  startRecentAbility(want, options, callback) {
    return this.__context_impl__.startRecentAbility(want, options, callback);
  }

  startAbilityWithAccount(want, accountId, options, callback) {
    return this.__context_impl__.startAbilityWithAccount(want, accountId, options, callback);
  }

  startAbilityByCall(want) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null) {
        hilog.sLogI(domainID, TAG, 'AbilityContext::startAbilityByCall input param error');
        reject(new ParamError('Parse param want failed, want must be Want'));
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want);
      } catch (error) {
        hilog.sLogI(domainID, TAG, 'AbilityContext::startAbilityByCall Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      hilog.sLogI(domainID, TAG, 'AbilityContext::startAbilityByCall success');
      return;
    });
  }

  startAbilityByCallWithAccount(want, accountId) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null || typeof accountId !== 'number') {
        hilog.sLogI(domainID, TAG, 'AbilityContext::startAbilityByCall With accountId input param error');
        reject(new ParamError(
          'Parse param want or accountId failed, want must be Want and accountId must be number'));
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want, accountId);
      } catch (error) {
        hilog.sLogI(domainID, TAG, 'AbilityContext::startAbilityByCall With accountId Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      hilog.sLogI(domainID, TAG, 'AbilityContext::startAbilityByCall With accountId success');
      return;
    });
  }

  startAbilityForResult(want, options, callback) {
    return this.__context_impl__.startAbilityForResult(want, options, callback);
  }

  startAbilityForResultWithAccount(want, accountId, options, callback) {
    return this.__context_impl__.startAbilityForResultWithAccount(want, accountId, options, callback);
  }

  startUIServiceExtensionAbility(want, callback) {
    return this.__context_impl__.startUIServiceExtensionAbility(want, callback);
  }

  connectUIServiceExtensionAbility(want, callback) {
    return this.__context_impl__.connectUIServiceExtensionAbility(want, callback);
  }

  disconnectUIServiceExtensionAbility(proxy) {
    return this.__context_impl__.disconnectUIServiceExtensionAbility(proxy);
  }

  startServiceExtensionAbility(want, callback) {
    return this.__context_impl__.startServiceExtensionAbility(want, callback);
  }

  startServiceExtensionAbilityWithAccount(want, accountId, callback) {
    return this.__context_impl__.startServiceExtensionAbilityWithAccount(want, accountId, callback);
  }

  stopServiceExtensionAbility(want, callback) {
    return this.__context_impl__.stopServiceExtensionAbility(want, callback);
  }

  stopServiceExtensionAbilityWithAccount(want, accountId, callback) {
    return this.__context_impl__.stopServiceExtensionAbilityWithAccount(want, accountId, callback);
  }

  connectServiceExtensionAbility(want, options) {
    return this.__context_impl__.connectServiceExtensionAbility(want, options);
  }

  connectAbilityWithAccount(want, accountId, options) {
    return this.__context_impl__.connectAbilityWithAccount(want, accountId, options);
  }

  connectServiceExtensionAbilityWithAccount(want, accountId, options) {
    return this.__context_impl__.connectServiceExtensionAbilityWithAccount(want, accountId, options);
  }

  disconnectAbility(connection, callback) {
    return this.__context_impl__.disconnectAbility(connection, callback);
  }

  disconnectServiceExtensionAbility(connection, callback) {
    return this.__context_impl__.disconnectServiceExtensionAbility(connection, callback);
  }

  terminateSelf(callback) {
    return this.__context_impl__.terminateSelf(callback);
  }

  isTerminating() {
    return this.__context_impl__.isTerminating();
  }

  terminateSelfWithResult(abilityResult, callback) {
    return this.__context_impl__.terminateSelfWithResult(abilityResult, callback);
  }

  backToCallerAbilityWithResult(abilityResult, requestCode) {
    return this.__context_impl__.backToCallerAbilityWithResult(abilityResult, requestCode);
  }

  restoreWindowStage(contentStorage) {
    return this.__context_impl__.restoreWindowStage(contentStorage);
  }

  setMissionContinueState(state, callback) {
    return this.__context_impl__.setMissionContinueState(state, callback);
  }

  setMissionLabel(label, callback) {
    return this.__context_impl__.setMissionLabel(label, callback);
  }

  setMissionIcon(icon, callback) {
    return this.__context_impl__.setMissionIcon(icon, callback);
  }

  setMissionWindowIcon(windowIcon) {
    return this.__context_impl__.setMissionWindowIcon(windowIcon);
  }

  setAbilityInstanceInfo(label, callback) {
    return this.__context_impl__.setAbilityInstanceInfo(label, callback);
  }

  requestDialogService(want, resultCallback) {
    return this.__context_impl__.requestDialogService(want, resultCallback);
  }

  reportDrawnCompleted(callback) {
    return this.__context_impl__.reportDrawnCompleted(callback);
  }

  startAbilityByType(type, wantParam, abilityStartCallback, callback) {
    return this.__context_impl__.startAbilityByType(type, wantParam, abilityStartCallback, callback);
  }

  requestModalUIExtension(want, callback) {
    return this.__context_impl__.requestModalUIExtension(want, callback);
  }

  requestModalUIExtensionWithAccount(want, accountId, callback) {
    return this.__context_impl__.requestModalUIExtensionWithAccount(want, accountId, callback);
  }

  showAbility() {
    return this.__context_impl__.showAbility();
  }

  hideAbility() {
    return this.__context_impl__.hideAbility();
  }

  openAtomicService(appId, options, callback) {
    return this.__context_impl__.openAtomicService(appId, options, callback);
  }

  moveAbilityToBackground(callback) {
    return this.__context_impl__.moveAbilityToBackground(callback);
  }

  setRestoreEnabled(enabled) {
    this.__context_impl__.setRestoreEnabled(enabled);
  }

  setColorMode(colorMode) {
    this.__context_impl__.setColorMode(colorMode);
  };

  revokeDelegator() {
    return this.__context_impl__.revokeDelegator();
  }

  startAppServiceExtensionAbility(want) {
    return this.__context_impl__.startAppServiceExtensionAbility(want);
  }

  stopAppServiceExtensionAbility(want) {
    return this.__context_impl__.stopAppServiceExtensionAbility(want);
  }

  connectAppServiceExtensionAbility(want, options) {
    return this.__context_impl__.connectAppServiceExtensionAbility(want, options);
  }

  disconnectAppServiceExtensionAbility(connection, callback) {
    return this.__context_impl__.disconnectAppServiceExtensionAbility(connection, callback);
  }

  setOnNewWantSkipScenarios(scenarios) {
    return this.__context_impl__.setOnNewWantSkipScenarios(scenarios);
  }

  startSelfUIAbilityInCurrentProcess(want, specifiedFlag, options) {
    return this.__context_impl__.startSelfUIAbilityInCurrentProcess(want, specifiedFlag, options);
  }

  restartApp(want) {
    return this.__context_impl__.restartApp(want);
  }

  startSelf() {
    return this.__context_impl__.startSelf();
  }
}

export default AbilityContext;
