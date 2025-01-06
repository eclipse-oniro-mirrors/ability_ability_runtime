/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';
import PositionUtils from '../utils/PositionUtils';
import deviceInfo from '@ohos.deviceInfo';

const TAG = 'TipsDialog_Service';

let tipsWinNum = 1;
let win;

export default class TipsServiceExtensionAbility extends extension {
  onCreate(want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    globalThis.tipsExtensionContext = this.context;
  }

  onRequest(want, startId) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    globalThis.abilityWant = want;
    try {
      globalThis.params = JSON.parse(want.parameters.params);
      globalThis.position = PositionUtils.getTipsDialogPosition();
      globalThis.callerToken = want.parameters.callerToken;
      display.on('change', (data: number) => {
        let tipsPosition = PositionUtils.getTipsDialogPosition();
        if (tipsPosition.offsetX !== globalThis.position.offsetX || tipsPosition.offsetY !== globalThis.position.offsetY) {
          win.moveTo(tipsPosition.offsetX, tipsPosition.offsetY);
        }
        if (tipsPosition.width !== globalThis.position.width || tipsPosition.height !== globalThis.position.height) {
          win.resetSize(tipsPosition.width, tipsPosition.height);
        }
        globalThis.position = tipsPosition;
      });
    } catch (exception) {
      console.error('Failed to register callback. Code: ' + JSON.stringify(exception));
    }

    display.getDefaultDisplay().then(dis => {
      let navigationBarRect = {
        left: globalThis.position.offsetX,
        top: globalThis.position.offsetY,
        width: globalThis.position.width,
        height: globalThis.position.height
      };
      if (tipsWinNum > 1) {
        win.destroy();
        tipsWinNum--;
      }
      let windowType = (typeof(globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
        window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
      this.createWindow('TipsDialog' + startId, windowType, navigationBarRect);
      tipsWinNum++;
    });
  }

  onDestroy() {
    console.info(TAG, 'TipsServiceExtensionAbility onDestroy.');
    if (win !== undefined) {
      win.destroy();
    }
  }

  private async createWindow(name: string, windowType: number, rect) {
    let deviceTypeInfo = deviceInfo.deviceType;
    console.info(TAG, 'create window');
    try {
      win = await window.create(globalThis.tipsExtensionContext, name, windowType);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        await win.bindDialogTarget(globalThis.callerToken.value, () => {
          win.destroyWindow();
          tipsWinNum--;
          if (tipsWinNum === 0) {
            globalThis.tipsExtensionContext.terminateSelf();
          }
        });
      }
      if (deviceTypeInfo !== 'default') {
        await win.hideNonSystemFloatingWindows(true);
      }
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      await win.loadContent('pages/tipsDialog');
      await win.setBackgroundColor('#00000000');
      await win.show();
    } catch {
      console.error(TAG, 'window create failed!');
    }
  }
};
