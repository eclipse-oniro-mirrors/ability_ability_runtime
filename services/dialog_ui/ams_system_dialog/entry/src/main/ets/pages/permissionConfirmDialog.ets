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

import router from '@ohos.router' 
import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import dataPreferences from '@ohos.data.preferences';
interface TipBtn {
  color:string
}
let storage = LocalStorage.GetShared();
@Entry(storage)
@Component
struct PermissionConfirmDialog {
  @State private btn: TipBtn = { color: '#FFFFFF' }
  private TAG = 'permissionconfirm_Page'

  onCloseApp() {
    console.info(this.TAG, 'click close app');
    if (globalThis.ExtensionType === 'ServiceExtension') {
        globalThis.currentExtensionContext.terminateSelf();
    } else {
        storage.get<UIExtensionContentSession>('session').terminateSelf();
    }
  }

  onConfirmApp() {
    console.info(this.TAG, 'click confirm app');
    globalThis.preferences.putSync('isdialogconfirmpermission', 'true')
    globalThis.preferences.flush();
    try {
      router.replaceUrl({
        url: globalThis.currentURL,
      })
    } catch (err) {
      console.error(this.TAG, 'Failed to replaceUrl. Cause: ' + JSON.stringify(err));
    }
  }

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Text($r('app.string.message_title_permission_confirm'))
          .fontSize(16)
          .fontWeight(FontWeight.Medium)
          .height('29%')
          .textOverflow({overflow: TextOverflow.Ellipsis})
        .textAlign(TextAlign.Center)
      }
      Flex({ justifyContent: FlexAlign.Center }) {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r('app.string.message_confirm_permission'))
            .fontSize('21fp')
            .fontColor('#0A59F7')
            .fontWeight(FontWeight.Regular)
            .textAlign(TextAlign.Center)
        }
        .width(175)
        .height(50)
        .borderRadius(28)
        .backgroundColor(this.btn.color)
        .onClick(() => {
          this.onConfirmApp();
        })
      }.margin({ top: 10})
      Flex({ justifyContent: FlexAlign.Center }) {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r('app.string.message_cancel_permission'))
            .fontSize('21fp')
            .fontColor('#0A59F7')
            .fontWeight(FontWeight.Regular)
            .textAlign(TextAlign.Center)
        }
        .width(175)
        .height(50)
        .borderRadius(28)
        .backgroundColor(this.btn.color)
        .onClick(() => {
          this.onCloseApp();
        })
      }.margin({ top: 10})
    }
    .borderRadius(20)
    .borderWidth(1)
    .borderColor('#e9e9e9')
    .backgroundColor('#FFFFFF')
  }
}