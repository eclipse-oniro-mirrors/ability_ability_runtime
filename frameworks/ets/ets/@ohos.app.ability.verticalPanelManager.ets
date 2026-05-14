/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import type common from '@ohos.app.ability.common';
import { AbilityResult } from 'ability.abilityResult';
import { BusinessError, RecordData } from '@ohos.base';

type OnErrorFn = (code: number, name: string, message: string) => void;

type OnResultFn = (parameter: AbilityResult) => void;

export default namespace verticalPanelManager {

  loadLibrary("vertical_panel_manager_ani_kit.z")

  export native function nativeStartVerticalPanelSync(
    context: common.UIAbilityContext,
    wantParam: Record<string, RecordData>,
    panelConfig: PanelConfig,
    panelStartCallback: PanelStartCallback): BusinessError;

  export function startVerticalPanel(
    context: common.UIAbilityContext,
    wantParam: Record<string, RecordData>,
    panelConfig: PanelConfig,
    panelStartCallback: PanelStartCallback
    ): Promise<void> {
    let p: Promise<void> = new Promise<void>((
      resolve: (v: undefined) => void, reject: (error: BusinessError) => void): void => {
      let p1 = taskpool.execute((): BusinessError => {
        return nativeStartVerticalPanelSync(context, wantParam, panelConfig, panelStartCallback);
      });
      p1.then((e: Any) => {
        let retError = e as BusinessError;
        if (retError.code === 0) {
          resolve(undefined);
        } else {
          reject(retError);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  export interface PanelConfig {
    type: VerticalType,
    sourceAppInfo: Record<string, string>
  }

  export enum VerticalType {
    NAVIGATION = 'navigation',
  }

  export interface PanelStartCallback {
    onError: OnErrorFn;
 
    onResult?: OnResultFn;
  }

  export const SOURCE_APP_BUNDLE_NAME: string = 'bundleName';

  export const SOURCE_APP_MODULE_NAME: string = 'moduleName';

  export const SOURCE_APP_ABILITY_NAME: string = 'abilityName';

  export const SOURCE_APP_WINDOW_ID: string = 'windowId';

  export const SOURCE_APP_SCREEN_MODE: string = 'screenMode';
}