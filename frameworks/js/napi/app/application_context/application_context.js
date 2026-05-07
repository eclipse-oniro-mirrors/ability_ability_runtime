/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
class BusinessError extends Error {
  constructor(code) {
    let msg = '';
    if (errMap.has(code)) {
      msg = errMap.get(code);
    } else {
      msg = ERROR_MSG_INNER_ERROR;
    }
    super(msg);
    this.code = code;
  }
}

class EventHub {
  constructor() {
    this.eventMap = {};
    this.nativeEventHubRef = null;
    this.contextIndex = -1;
    this.emitMultiThreadingEnabled = false;
    this.emitter = undefined;
  }

  setNativeEventHubRef(ref) {
    this.nativeEventHubRef = ref;
  }

  onEmitterFunction(eventData = {'data':{}}) {
    let eventName = eventData.data[`arg${0}`];
    let arrays = Object.keys(eventData.data);
    let result = {};
    for (let index = 1; index < arrays.length; index++) {
      result[`arg${index - 1}`] = eventData.data[`arg${index}`];
    }
    let args = this.dataToArgs(result);
    this.emitInner(eventName, ...args);
  }

  getEmitterEventName(event) {
    return 'ohos.event.' + this.contextIndex + '.' + event;
  }

  argsToData(args = []) {
    let eventData = {};
    let data = args.reduce((acc, value, index)=>{
      acc[`arg${index}`] = value;
      return acc;
    }, {});
    eventData.data = data;
    return eventData;
  }

  dataToArgs(data = {}) {
    if (!data) {
      return [];
    }
    let args = [];
    let oLen = Object.keys(data).length;
    for (let index = 0; index < oLen; index++) {
      args[index] = data[`arg${index}`];
    }
    return args;
  }

  setEventHubEmitMultiThreadingEnabled(enable) {
    if (this.emitMultiThreadingEnabled === enable) {
      return;
    }
    this.emitter = requireNapi('events.emitter');
    this.emitMultiThreadingEnabled = enable;
    let keys = Object.keys(this.eventMap);
    for (let i = 0; i < keys.length; i++) {
      if (this.emitMultiThreadingEnabled === true) {
        this.emitter.on(this.getEmitterEventName(keys[i]), (eventData) => this.onEmitterFunction(eventData));
      } else {
        this.emitter.off(this.getEmitterEventName(keys[i]));
      }
    }
  }

  on(event, callback) {
    if ((typeof (event) !== 'string') || (typeof (callback) !== 'function')) {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (!this.eventMap[event]) {
      this.eventMap[event] = [];
    }
    if (this.eventMap[event].indexOf(callback) === -1) {
      this.eventMap[event].push(callback);
    }
    if (this.emitMultiThreadingEnabled === true && this.emitter !== undefined) {
      if (this.emitter.getListenerCount(this.getEmitterEventName(event)) === 0) {
        this.emitter.on(this.getEmitterEventName(event), (eventData) => this.onEmitterFunction(eventData));
      }
    }
  }

  offByNativeContext(event, callback) {
    if (typeof (event) !== 'string') {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (this.eventMap[event]) {
      if (callback) {
        let cbArray = this.eventMap[event];
        let index = cbArray.indexOf(callback);
        if (index > -1) {
          for (; index + 1 < cbArray.length; index++) {
            cbArray[index] = cbArray[index + 1];
          }
          cbArray.pop();
        }
      } else {
        delete this.eventMap[event];
      }
    }
  }

  off(event, callback) {
    if (typeof (event) !== 'string') {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (this.eventMap[event]) {
      if (callback) {
        let cbArray = this.eventMap[event];
        let index = cbArray.indexOf(callback);
        if (index > -1) {
          for (; index + 1 < cbArray.length; index++) {
            cbArray[index] = cbArray[index + 1];
          }
          cbArray.pop();
        }
      } else {
        delete this.eventMap[event];
      }
    }
    if (this.nativeEventHubRef != null) {
      if (callback) {
        this.nativeEventHubRef.offByDynamicContext(event, callback);
      } else {
        this.nativeEventHubRef.offByDynamicContext(event);
      }
    }
    if (this.emitMultiThreadingEnabled === true && this.emitter !== undefined) {
      let array = this.eventMap[event];
      if (array === null || array === undefined) {
        this.emitter.off(this.getEmitterEventName(event));
      }
    }
  }

  emit(event, ...args) {
    if (this.emitMultiThreadingEnabled === true && this.emitter !== undefined) {
      let eventNameArrays = [event];
      let newArgsArrays = eventNameArrays.concat(args);
      let eventData = this.argsToData(newArgsArrays);
      this.emitter.emit(this.getEmitterEventName(event), eventData);
    } else {
      this.emitInner(event, ...args);
    }
  }

  emitInner(event, ...args) {
    if (typeof (event) !== 'string') {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (this.eventMap[event]) {
      const cloneArray = [...this.eventMap[event]];
      const len = cloneArray.length;
      for (let i = 0; i < len; ++i) {
        cloneArray[i].apply(this, args);
      }
    }
    if (this.nativeEventHubRef != null) {
      this.nativeEventHubRef.emitByDynamicContext(event, ...args);
    }
  }

  emitByNativeContext(event, ...args) {
    if (typeof (event) !== 'string') {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (this.eventMap[event]) {
      const cloneArray = [...this.eventMap[event]];
      const len = cloneArray.length;
      for (let i = 0; i < len; ++i) {
        cloneArray[i].apply(this, args);
      }
    }
  }
}

class ApplicationContext {
  static contextTypeMap = {
    0: 'ApplicationContext',
    1: 'AbilityStageContext',
    2: 'UIAbilityContext',
    3: 'FormExtensionContext',
    4: 'AppServiceExtensionContext',
    5: 'ServiceExtensionContext',
    6: 'UIServiceExtensionContext',
    7: 'AutoFillExtensionContext'
  };

  constructor(obj) {
    this.__context_impl__ = obj;
    let eventHub = new EventHub();
    eventHub.contextIndex = obj.index;
    this.__context_impl__.eventHub = eventHub;
  }

  isContextOf(contextType) {
    if (typeof (contextType) !== 'number') {
      return false;
    }
    const expectedType = ApplicationContext.contextTypeMap[contextType];
    if (!expectedType) {
      return false;
    }
    return this.__context_impl__.contextType === expectedType;
  }

  registerAbilityLifecycleCallback(abilityLifecycleCallback) {
    return this.__context_impl__.registerAbilityLifecycleCallback(abilityLifecycleCallback);
  }

  unregisterAbilityLifecycleCallback(callbackId, callback) {
    return this.__context_impl__.unregisterAbilityLifecycleCallback(callbackId, callback);
  }

  registerEnvironmentCallback(environmentCallback) {
    return this.__context_impl__.registerEnvironmentCallback(environmentCallback);
  }

  unregisterEnvironmentCallback(callbackId, envcallback) {
    return this.__context_impl__.unregisterEnvironmentCallback(callbackId, envcallback);
  }

  on(type, callback) {
    return this.__context_impl__.on(type, callback);
  }

  off(type, callbackId, callback) {
    return this.__context_impl__.off(type, callbackId, callback);
  }

  createBundleContext(bundleName) {
    return this.__context_impl__.createBundleContext(bundleName);
  }

  createModuleContext(moduleName) {
    return this.__context_impl__.createModuleContext(moduleName);
  }

  createModuleContext(bundleName, moduleName) {
    return this.__context_impl__.createModuleContext(bundleName, moduleName);
  }

  createSystemHspModuleResourceManager(bundleName, moduleName) {
    return this.__context_impl__.createSystemHspModuleResourceManager(bundleName, moduleName);
  }

  createModuleResourceManager(bundleName, moduleName) {
    return this.__context_impl__.createModuleResourceManager(bundleName, moduleName);
  }

  getGroupDir(groupId, callback) {
    return this.__context_impl__.getGroupDir(groupId, callback);
  }

  getApplicationContext() {
    return this.__context_impl__.getApplicationContext();
  }

  killAllProcesses(callback) {
    return this.__context_impl__.killAllProcesses(callback);
  }

  getProcessRunningInformation(callback) {
    return this.__context_impl__.getProcessRunningInformation(callback);
  }

  getRunningProcessInformation(callback) {
    return this.__context_impl__.getProcessRunningInformation(callback);
  }

  setColorMode(colorMode) {
    return this.__context_impl__.setColorMode(colorMode);
  }

  onSystemConfigurationUpdated(colorMode) {
    return this.__context_impl__.onSystemConfigurationUpdated(colorMode);
  }

  offSystemConfigurationUpdated(colorMode) {
    return this.__context_impl__.offSystemConfigurationUpdated(colorMode);
  }

  setLanguage(language) {
    return this.__context_impl__.setLanguage(language);
  }

  setFont(font) {
    return this.__context_impl__.setFont(font);
  }

  setFontSizeScale(fontSizeScale) {
    return this.__context_impl__.setFontSizeScale(fontSizeScale);
  }

  setAutoStartup(info, callback) {
    return this.__context_impl__.setAutoStartup(info, callback);
  }

  cancelAutoStartup(info, callback) {
    return this.__context_impl__.cancelAutoStartup(info, callback);
  }

  isAutoStartup(info, callback) {
    return this.__context_impl__.isAutoStartup(info, callback);
  }

  clearUpApplicationData() {
    return this.__context_impl__.clearUpApplicationData();
  }

  clearUpApplicationData(callback) {
    return this.__context_impl__.clearUpApplicationData(callback);
  }

  preloadUIExtensionAbility(want) {
    return this.__context_impl__.preloadUIExtensionAbility(want);
  }

  restartApp(want) {
    return this.__context_impl__.restartApp(want);
  }

  setSupportedProcessCache(isSupport) {
    return this.__context_impl__.setSupportedProcessCache(isSupport);
  }

  getCurrentAppCloneIndex() {
    return this.__context_impl__.getCurrentAppCloneIndex();
  }

  getCurrentInstanceKey() {
    return this.__context_impl__.getCurrentInstanceKey();
  }

  getAllRunningInstanceKeys() {
    return this.__context_impl__.getAllRunningInstanceKeys();
  }

  createAreaModeContext(areaMode) {
    return this.__context_impl__.createAreaModeContext(areaMode);
  }

  createDisplayContext(displayId) {
    return this.__context_impl__.createDisplayContext(displayId);
  }

  getAllWindowStages() {
    return this.__context_impl__.getAllWindowStages();
  }

  getUIAbilityByInstanceId(instanceId) {
    return this.__context_impl__.getUIAbilityByInstanceId(instanceId);
  }

  set area(mode) {
    return this.__context_impl__.switchArea(mode);
  }

  get area() {
    return this.__context_impl__.getArea();
  }

  get resourceManager() {
    return this.__context_impl__.resourceManager;
  }

  get applicationInfo() {
    return this.__context_impl__.applicationInfo;
  }

  get cacheDir() {
    return this.__context_impl__.cacheDir;
  }

  get tempDir() {
    return this.__context_impl__.tempDir;
  }

  get resourceDir() {
    return this.__context_impl__.resourceDir;
  }

  get filesDir() {
    return this.__context_impl__.filesDir;
  }

  get distributedFilesDir() {
    return this.__context_impl__.distributedFilesDir;
  }

  get databaseDir() {
    return this.__context_impl__.databaseDir;
  }

  get preferencesDir() {
    return this.__context_impl__.preferencesDir;
  }

  get bundleCodeDir() {
    return this.__context_impl__.bundleCodeDir;
  }

  get cloudFileDir() {
    return this.__context_impl__.cloudFileDir;
  }

  get logFileDir() {
    return this.__context_impl__.logFileDir;
  }

  get eventHub() {
    return this.__context_impl__.eventHub;
  }

  get processName() {
    return this.__context_impl__.processName;
  }

  get stageMode() {
    return true;
  }
}

export default ApplicationContext;
