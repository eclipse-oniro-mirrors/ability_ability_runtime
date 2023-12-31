/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
class ParamError extends Error {
  constructor() {
    super(ERROR_MSG_INVALID_PARAM);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class AbilityContext extends Context {
  constructor(obj) {
    super(obj);
    this.abilityInfo = obj.abilityInfo;
    this.currentHapModuleInfo = obj.currentHapModuleInfo;
    this.config = obj.config;
  }

  onUpdateConfiguration(config) {
    this.config = config;
  }

  startAbility(want, options, callback) {
    return this.__context_impl__.startAbility(want, options, callback);
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
        console.log('AbilityContext::startAbilityByCall input param error');
        reject(new ParamError());
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want);
      } catch (error) {
        console.log('AbilityContext::startAbilityByCall Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      console.log('AbilityContext::startAbilityByCall success');
      return;
    });
  }

  startAbilityByCallWithAccount(want, accountId) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null || typeof accountId !== 'number') {
        console.log('AbilityContext::startAbilityByCall With accountId input param error');
        reject(new ParamError());
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want, accountId);
      } catch (error) {
        console.log('AbilityContext::startAbilityByCall With accountId Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      console.log('AbilityContext::startAbilityByCall With accountId success');
      return;
    });
  }

  startAbilityForResult(want, options, callback) {
    return this.__context_impl__.startAbilityForResult(want, options, callback);
  }

  startAbilityForResultWithAccount(want, accountId, options, callback) {
    return this.__context_impl__.startAbilityForResultWithAccount(want, accountId, options, callback);
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

  connectAbility(want, options) {
    return this.__context_impl__.connectAbility(want, options);
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

  restoreWindowStage(contentStorage) {
    return this.__context_impl__.restoreWindowStage(contentStorage);
  }

  setMissionLabel(label, callback) {
    return this.__context_impl__.setMissionLabel(label, callback);
  }

  setMissionIcon(icon, callback) {
    return this.__context_impl__.setMissionIcon(icon, callback);
  }

  requestDialogService(want, resultCallback) {
    return this.__context_impl__.requestDialogService(want, resultCallback);
  }

  reportDrawnCompleted(callback) {
    return this.__context_impl__.reportDrawnCompleted(callback);
  }
}

export default AbilityContext;
