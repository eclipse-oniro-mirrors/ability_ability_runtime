/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import bundleManager from '@ohos.bundle.bundleManager';
import defaultAppManager from '@ohos.bundle.defaultAppManager';
import display from '@ohos.display';
import drawableDescriptor from '@ohos.arkui.drawableDescriptor';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import type image from '@ohos.multimedia.image';
import window from '@ohos.window';
import PositionUtils from '../utils/PositionUtils';
import deviceInfo from '@ohos.deviceInfo';
import systemparameter from '@ohos.systemParameterEnhance';
import dataPreferences from '@ohos.data.preferences';

const TAG = 'SelectorDialog_Service';

let winNum = 1;
let win;

export default class SelectorServiceExtensionAbility extends extension {
  onCreate(want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    globalThis.selectExtensionContext = this.context;
    globalThis.currentExtensionContext = this.context;
    globalThis.ExtensionType = 'ServiceExtension';
    globalThis.defaultAppManager = defaultAppManager;
    globalThis.bundleManager = bundleManager;
    let options = {name:'dialogStore'};
    try {
      globalThis.preferences = dataPreferences.getPreferencesSync(this.context, options);
    } catch (error) {
      console.error(TAG, 'Failed to getPreferencesSync callback. Code: ' + JSON.stringify(error));
    }
  }

  async getPhoneShowHapList() {
    const lineNums = 8;
    let showHapList = [];
    let phoneShowHapList = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    for (let i = 1; i <= globalThis.params.hapList.length; i++) {
      console.info(TAG, 'hapList[' + (i - 1).toString() + ']: ' + JSON.stringify(globalThis.params.hapList[i - 1]));
      await this.getHapResource(globalThis.params.hapList[i - 1], showHapList, jsonIconMap);
      if (i % lineNums === 0) {
        phoneShowHapList.push(showHapList);
        showHapList = [];
      }
      if (i >= globalThis.params.hapList.length && showHapList.length > 0) {
        phoneShowHapList.push(showHapList);
      }
    }
    globalThis.phoneShowHapList = phoneShowHapList;
    console.debug(TAG, 'phoneShowHapList: ' + JSON.stringify(phoneShowHapList));

    const signalRowlineNums = 4;
    let signalRowShowHapList = [];
    let signalRowPhoneShowHapList = [];
    for (let i = 1; i <= globalThis.params.hapList.length; i++) {
      console.info(TAG, 'hapList[' + (i - 1).toString() + ']: ' + JSON.stringify(globalThis.params.hapList[i - 1]));
      await this.getHapResource(globalThis.params.hapList[i - 1], signalRowShowHapList, jsonIconMap);
      if (i % signalRowlineNums === 0) {
        signalRowPhoneShowHapList.push(signalRowShowHapList);
        signalRowShowHapList = [];
      }
      if (i >= globalThis.params.hapList.length && signalRowShowHapList.length > 0) {
        signalRowPhoneShowHapList.push(signalRowShowHapList);
      }
    }
    globalThis.signalRowPhoneShowHapList = signalRowPhoneShowHapList;
    globalThis.jsonIconMap = jsonIconMap;
    console.debug(TAG, 'signalRowPhoneShowHapList: ' + JSON.stringify(signalRowPhoneShowHapList));
  }

  async getPcShowHapList() {
    let pcShowHapList = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    for (let i = 0; i < globalThis.params.hapList.length; i++) {
      await this.getHapResource(globalThis.params.hapList[i], pcShowHapList, jsonIconMap);
    }
    globalThis.pcShowHapList = pcShowHapList;
    globalThis.jsonIconMap = jsonIconMap;
    console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(pcShowHapList));
  }

  async getHapResource(hap, showHapList, jsonIconMap) {
    let bundleName = hap.bundle;
    let moduleName = hap.module;
    let abilityName = hap.ability;
    let appName = '';
    let appIcon = '';
    let type = '';
    let userId = Number('0');
    let appIndex = Number(hap.appIndex);
    if (!globalThis.params.isDefaultSelector) {
      type = hap.type;
      userId = Number(hap.userId);
    }
    let lableId = Number(hap.label);
    if (lableId === 0) {
      lableId = Number(hap.bundleLabel);
    }
    let moduleContext = globalThis.selectExtensionContext.createModuleContext(bundleName, moduleName);
    await moduleContext.resourceManager.getString(lableId).then(value => {
      if (appIndex === 0) {
        appName = value;
      } else {
        appName = value + hap.appIndex;
      }
    }).catch(error => {
      console.error(TAG, 'getString error:' + JSON.stringify(error));
    });

    let iconId = Number(hap.icon);
    if (iconId === 0) {
      iconId = Number(hap.bundleIcon);
    }
    await moduleContext.resourceManager.getMediaBase64(iconId).then(value => {
      appIcon = value;
      if (appIcon.indexOf('image/json') > -1) {
        try {
          const imageDescriptor = moduleContext.resourceManager.getDrawableDescriptor(iconId);
          if (imageDescriptor !== null && imageDescriptor !== undefined &&
            imageDescriptor instanceof drawableDescriptor.LayeredDrawableDescriptor) {
            let layeredDrawableDescriptor: drawableDescriptor.LayeredDrawableDescriptor =
              <drawableDescriptor.LayeredDrawableDescriptor> imageDescriptor;
            let foregroundDescriptor: drawableDescriptor.DrawableDescriptor = layeredDrawableDescriptor.getForeground();
            if (foregroundDescriptor !== null && foregroundDescriptor !== undefined) {
              jsonIconMap.set(bundleName + ':' + moduleName + ':' + abilityName + ':' + hap.appIndex,
                foregroundDescriptor.getPixelMap());
            } else {
              console.error(TAG, 'get foregroundDescriptor is null');
            }
          }
        } catch (e) {
          console.error(TAG, 'get drawableDescriptor error:' + JSON.stringify(e));
        }
      }
    }).catch(error => {
      console.error(TAG, 'getMediaBase64 error:' + JSON.stringify(error));
    });
    showHapList.push(bundleName + '#' + abilityName + '#' + appName +
      '#' + appIcon + '#' + moduleName + '#' + type + '#' + userId + '#' + appIndex);
  }

  async onRequest(want, startId) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    globalThis.abilityWant = want;
    try {
      globalThis.params = JSON.parse(want.parameters.params);
      globalThis.callerToken = want.parameters.callerToken;
      let lineNums = 0;
      if (globalThis.params && globalThis.params.hapList && globalThis.params.hapList.length) {
        lineNums = globalThis.params.hapList.length;
      }
      globalThis.position = PositionUtils.getSelectorDialogPosition(lineNums);
      display.on('change', (data: number) => {
        let position = PositionUtils.getSelectorDialogPosition(lineNums);
        if (position.offsetX !== globalThis.position.offsetX || position.offsetY !== globalThis.position.offsetY) {
          win.moveTo(position.offsetX, position.offsetY);
        }
        if (position.width !== globalThis.position.width || position.height !== globalThis.position.height) {
          win.resetSize(position.width, position.height);
        }
        globalThis.position = position;
      });
    } catch (exception) {
      console.error('Failed to register callback. Code: ' + JSON.stringify(exception));
    }
    try {
      let displayClass = display.getDefaultDisplaySync();
      console.debug(TAG, 'onRequest display is' + JSON.stringify(displayClass));
    } catch (error) {
      console.error(TAG, 'Failed to getDefaultDisplaySync callback. Code: ' + JSON.stringify(error));
    }
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
    console.debug(TAG, 'onRequest, position: ' + JSON.stringify(globalThis.position));
    if (!globalThis.params.isDefaultSelector) {
      globalThis.modelFlag = Boolean(globalThis.params.modelFlag);
      globalThis.action = Boolean(globalThis.params.action);
    }
    if (globalThis.params.isDefaultSelector) {
      await this.getPhoneShowHapList();
    } else {
      await this.getPcShowHapList();
    }

    AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
    display.getDefaultDisplay().then(dis => {
      let navigationBarRect = {
        left: globalThis.position.offsetX,
        top: globalThis.position.offsetY,
        width: globalThis.position.width,
        height: globalThis.position.height
      };
      if (winNum > 1) {
        win.destroy();
        winNum--;
      }
      let windowType = (typeof(globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
        window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
      this.createWindow('SelectorDialog' + startId, windowType, navigationBarRect);
      winNum++;
    });
  }

  onDestroy() {
    console.info(TAG, 'SelectorServiceExtensionAbility onDestroy.');
    if (win !== undefined) {
      win.destroy();
    }
  }

  private async createWindow(name: string, windowType: number, rect) {
    let deviceTypeInfo = deviceInfo.deviceType;
    console.info(TAG, 'create window');
    try {
      win = await window.create(globalThis.selectExtensionContext, name, windowType);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        await win.bindDialogTarget(globalThis.callerToken.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            globalThis.selectExtensionContext.terminateSelf();
          }
        });
      }
      if (deviceTypeInfo !== 'default') {
        await win.hideNonSystemFloatingWindows(true);
      }
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      if (systemparameter.getSync('persist.sys.abilityms.isdialogconfirmpermission', 'false') === 'false' &&
        globalThis.preferences.getSync('isdialogconfirmpermission', 'false') === 'false') {
        if (globalThis.params.isDefaultSelector) {
          globalThis.currentURL = 'pages/selectorPhoneDialog';
        } else {
          globalThis.currentURL = 'pages/selectorPcDialog';
        }
        await win.loadContent('pages/permissionConfirmDialog');
      } else {
        if (globalThis.params.isDefaultSelector) {
          await win.loadContent('pages/selectorPhoneDialog');
        } else {
          await win.loadContent('pages/selectorPcDialog');
        }
      }
      await win.setBackgroundColor('#00000000');
      await win.show();
    } catch (e) {
      console.error(TAG, 'window create failed: ' + JSON.stringify(e));
    }
  }

  private async moveWindow(rect): Promise<void> {
    try {
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      if (globalThis.params.isDefaultSelector) {
        try {
          await win.loadContent('pages/selectorPhoneDialog');
          await win.setBackgroundColor('#00000000');
        } catch (e) {
          console.error(TAG, 'window loadContent failed: ' + JSON.stringify(e));
        }
      }
      await win.show();
    } catch (e) {
      console.error(TAG, 'window move failed: ' + JSON.stringify(e));
    }
  }

  onConfigurationUpdate(config): void {
    console.debug(TAG, 'configuration is : ' + JSON.stringify(config));
    if (!globalThis.params || !globalThis.params.isDefaultSelector) {
      console.debug(TAG, 'device is not phone');
      return;
    }
    try {
      let displayClass = display.getDefaultDisplaySync();
      console.debug(TAG, 'display is' + JSON.stringify(displayClass));
      if (displayClass.orientation === display.Orientation.PORTRAIT || displayClass.orientation === display.Orientation.PORTRAIT_INVERTED) {
        globalThis.position = globalThis.verticalPosition;
      } else {
        globalThis.position = globalThis.landScapePosition;
      }
    } catch (error) {
      console.error(TAG, 'Failed to getDefaultDisplaySync callback. Code: ' + JSON.stringify(error));
    }
    if (!globalThis.position) {
      console.error(TAG, 'globalThis.position is invalid, skip moveWindow');
      return;
    }
    let navigationBarRect = {
      left: globalThis.position.offsetX,
      top: globalThis.position.offsetY,
      width: globalThis.position.width,
      height: globalThis.position.height
    };
    AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
    console.debug(TAG, 'onConfigurationUpdate navigationBarRect is' + JSON.stringify(navigationBarRect));
    this.moveWindow(navigationBarRect);
  }
};