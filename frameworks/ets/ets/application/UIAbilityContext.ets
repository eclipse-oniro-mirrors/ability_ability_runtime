/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { AbilityInfo } from 'bundleManager.AbilityInfo';
import Want from '@ohos.app.ability.Want';
import StartOptions from '@ohos.app.ability.StartOptions';
import { Caller } from '@ohos.app.ability.UIAbility';
import { BusinessError, AsyncCallback, RecordData } from '@ohos.base';
import { AbilityResult } from 'ability.abilityResult';
import { Configuration } from '@ohos.app.ability.Configuration';
import { ConnectOptions } from 'ability.connectOptions';
import Context from 'application.Context';
import window from '@ohos.window';
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import OpenLinkOptions from '@ohos.app.ability.OpenLinkOptions';
import { HapModuleInfo } from 'bundleManager.HapModuleInfo';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import AbilityStartCallback from 'application.AbilityStartCallback';
import AtomicServiceOptions from '@ohos.app.ability.AtomicServiceOptions';
import UIServiceProxy from 'application.UIServiceProxy';
import UIServiceExtensionConnectCallback from 'application.UIServiceExtensionConnectCallback';
import dialogRequest from '@ohos.app.ability.dialogRequest';
import image from '@ohos.multimedia.image';
import { LocalStorage } from '@ohos.arkui.stateManagement';
import AbilityConstant  from '@ohos.app.ability.AbilityConstant';

export class Cleaner {
  public ptr: long = 0;

  constructor(ptr: long) {
    this.ptr = ptr;
  }

  native clean(): void;
}

export function callback(cleaner: Cleaner): void {
  cleaner.clean();
}

let destroyRegister = new FinalizationRegistry<Cleaner>(callback);
let unregisterToken = new object();

export default class UIAbilityContext extends Context {
  static {
    loadLibraryWithPermissionCheck("ability_context_ani_kit.z", "application.UIAbilityContext");
  }

  private cleaner: Cleaner | null = null;
  etsAbilityContextPtr: long = 0;
  config!: Configuration;
  abilityInfo!: AbilityInfo;
  windowStage!: window.WindowStage;
  currentHapModuleInfo!: HapModuleInfo;

  private setEtsAbilityContextPtr(ptr: long) {
    if (this.etsAbilityContextPtr == 0) {
      this.etsAbilityContextPtr = ptr;
    }
    this.registerCleaner(this.etsAbilityContextPtr);
  }

  registerCleaner(ptr: long): void {
    this.cleaner = new Cleaner(ptr);
    destroyRegister.register(this, this.cleaner!, unregisterToken);
  }

  unregisterCleaner(): void {
    destroyRegister.unregister(unregisterToken);
  }

  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'UIAbilityContext';

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    console.log('transferStatic type: ' + type);
    return UIAbilityContext.nativeTransferStatic(ESValue.wrap(input), type);
  }
  static transferDynamic(input: Object): Any {
    return UIAbilityContext.nativeTransferDynamic(input).unwrap();
  }

  private native nativeStartAbilitySync(want: Want, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAbilitySyncCheck(options: StartOptions): void;

  private native nativeStartAbilitySync(want: Want, options: StartOptions, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAbilityForResult(want: Want, callback: AsyncCallbackWrapper<AbilityResult>): void;

  private native nativeStartAbilityForResult(want: Want, startOptions: StartOptions,
    callback: AsyncCallbackWrapper<AbilityResult>): void;

  private native nativeStartAbilityByCallWithAccountSync(want: Want, accountId: int): Caller;

  private native nativeTerminateSelfSync(callback: AsyncCallbackWrapper<void>): void;

  private native nativeTerminateSelfWithResult(parameter: AbilityResult, callback: AsyncCallbackWrapper<void>): void;

  private native nativeReportDrawnCompletedSync(callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  private native nativeOpenLink(link: string, myCallback:AsyncCallbackWrapper<void>, options?: OpenLinkOptions, callback?: AsyncCallbackWrapper<AbilityResult>): void;

  private native nativeIsTerminating(): boolean;

  private native nativeMoveAbilityToBackground(callback: AsyncCallbackWrapper<void>): void;

  private native nativeRequestModalUIExtension(pickerWant: Want, callback: AsyncCallbackWrapper<void>): void;

  private native nativeRequestModalUIExtensionWithAccount(
    pickerWant: Want, accountId: int, callback: AsyncCallbackWrapper<void>): void;

  private native nativeBackToCallerAbilityWithResult(abilityResult: AbilityResult, requestCode: string, callback: AsyncCallbackWrapper<void>): void;

  private native nativeSetMissionLabel(label: string, callback: AsyncCallbackWrapper<void>): void;

  private native nativeConnectServiceExtensionAbility(want: Want, options: ConnectOptions): long;

  private native nativeDisconnectServiceExtensionAbility(connection: long, callback: AsyncCallbackWrapper<void>): void;

  private native nativeSetColorMode(colorMode: ConfigurationConstant.ColorMode): void;

  private native nativeChangeAbilityVisibility(isShow: boolean, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAbilityByTypeSync(type: string, wantParam: Record<string, RecordData>,
    startCallback: AbilityStartCallback): BusinessError;
 
  private native nativeRequestDialogService(want: Want,
    result: AsyncCallbackWrapper<dialogRequest.RequestResult>): void;

  private native nativeSetAbilityInstanceInfo(label: string, icon: image.PixelMap, callback: AsyncCallbackWrapper<void>): void;

  private native nativeOpenAtomicService(appId: string, callback: AsyncCallbackWrapper<AbilityResult>, options?: AtomicServiceOptions): void;

  private native nativeStartSelfUIAbilityInCurrentProcessSync(want: Want, specifiedFlag: string, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartSelfUIAbilityInCurrentProcessSync(want: Want, specifiedFlag: string, options: StartOptions, callback: AsyncCallbackWrapper<void>): void;

  private native nativeOnSetRestoreEnabled(enabled: boolean): void;

  private native nativeConnectServiceExtensionAbilityWithAccount(want: Want, accountId: int,
    options: ConnectOptions): long;

  private native nativeStopServiceExtensionAbilityWithAccount(want: Want, accountId: int,
    callback: AsyncCallbackWrapper<void>): void;

  private native nativeStopServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartServiceExtensionAbilityWithAccount(want: Want, accountId: int,
    callback: AsyncCallbackWrapper<void>): void;

  private native nativeSetMissionIcon(icon: image.PixelMap, callback: AsyncCallbackWrapper<void>): void;

  private native nativeSetMissionContinueState(state: AbilityConstant.ContinueState, callback: AsyncCallbackWrapper<void>): void;

  private native nativeRevokeDelegator(callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAppServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStopAppServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  private native nativeConnectAppServiceExtensionAbility(want: Want, options: ConnectOptions): long;

  private native nativeDisconnectAppServiceExtensionAbility(connection: long, callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAbilityForResultWithAccount(want: Want, accountId: int,
    callback: AsyncCallbackWrapper<AbilityResult>): void;

  private native nativeStartAbilityForResultWithAccountVoid(want: Want, accountId: int, options: StartOptions,
    callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAbilityForResultWithAccountResult(want: Want, accountId: int, options: StartOptions,
    callback: AsyncCallbackWrapper<AbilityResult>): void;

  private native nativeStartAbilityWithAccountSync(want: Want, accountId: int,
    callback: AsyncCallbackWrapper<void>): void;

  private native nativeStartAbilityWithAccountSync(want: Want, accountId: int, options: StartOptions,
    callback: AsyncCallbackWrapper<void>): void;

  private native nativeRestoreWindowStage(localStorage: LocalStorage): void;

  private native nativeStartAbilityAsCaller(want: Want, callback: AsyncCallbackWrapper<void>, options?: StartOptions): void;

  private native nativeStartRecentAbility(want: Want, callback: AsyncCallbackWrapper<void>, options?: StartOptions): void;
  
  private native nativeOpenLinkCheck(link: string): void;

  private native nativeOpenAtomicServiceCheck(): void;

  private native nativeSetAbilityInstanceInfoCheck(label: string, icon: image.PixelMap): void;

  private native nativeSetMissionIconCheck(icon: image.PixelMap): void;

  private native nativeStartAbilityForResultWithAccountCheck(): void;
  private native nativeSetMissionWindowIcon(windowIcon: image.PixelMap, callback: AsyncCallbackWrapper<void>): void;

  private native nativeSetOnNewWantSkipScenarios(scenarios: int, callback: AsyncCallbackWrapper<void>): void;

  startAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilitySync(want, myCall);
    });
  }

  private native nativeWantCheck(want: Want): void;
  private native nativeDisconnectUIServiceExtensionCheck(proxy: UIServiceProxy) : void;
  private native nativeConnectUIServiceExtensionAbility(want: Want, uiServiceExtConCallback: UIServiceExtensionConnectCallback, callback:AsyncCallbackWrapper<UIServiceProxy>): void;
  private native nativeStartUIServiceExtensionAbility(want: Want, callback:AsyncCallbackWrapper<void>): void;
  private native nativeDisconnectUIServiceExtensionAbility(proxy: UIServiceProxy, callback:AsyncCallbackWrapper<void>): void;

  private native nativeRestartAppSync(want: Want): void;

  startAbility(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    this.nativeStartAbilitySyncCheck(options);
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilitySync(want, options, myCall);
    });
  }

  startAbility(want: Want, options?: StartOptions): Promise<void> {
    if (options != null) {
      this.nativeStartAbilitySyncCheck(options);
    } 
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (options == null) {
          this.nativeStartAbilitySync(want, myCall);
        } else {
          this.nativeStartAbilitySync(want, options as StartOptions, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startAbilityForResult(want: Want, callback: AsyncCallback<AbilityResult, void>): void {
    let myCall = new AsyncCallbackWrapper<AbilityResult>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityForResult(want, myCall);
    });
  }

  startAbilityForResult(want: Want, startOptions: StartOptions, callback: AsyncCallback<AbilityResult, void>): void {
    let myCall = new AsyncCallbackWrapper<AbilityResult>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityForResult(want, startOptions, myCall);
    });
  }

  startAbilityForResult(want: Want, startOptions?: StartOptions): Promise<AbilityResult> {
    return new Promise<AbilityResult>((resolve: (data: AbilityResult) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall =
        new AsyncCallbackWrapper<AbilityResult>((err: BusinessError | null, data: AbilityResult | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as AbilityResult);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        if (startOptions == null) {
          this.nativeStartAbilityForResult(want, myCall);
        } else {
          this.nativeStartAbilityForResult(want, startOptions as StartOptions, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startAbilityByCall(want: Want): Promise<Caller> {
    return this.startAbilityByCallWithAccount(want, -1);
  }

  startAbilityByCallWithAccount(want: Want, accountId: int): Promise<Caller> {
    return taskpool.execute((): Caller => {
      return this.nativeStartAbilityByCallWithAccountSync(want, accountId);
    }) as Promise<Caller>;
  }

  terminateSelf(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeTerminateSelfSync(myCall);
    });
  }

  terminateSelf(): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeTerminateSelfSync(myCall);
        });
      });
    return p;
  }

  terminateSelfWithResult(parameter: AbilityResult, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeTerminateSelfWithResult(parameter, myCall);
    });
  }

  terminateSelfWithResult(parameter: AbilityResult): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeTerminateSelfWithResult(parameter, myCall);
        });
      });
    return p;
  }

  reportDrawnCompleted(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeReportDrawnCompletedSync(myCall);
    });
  }

  startServiceExtensionAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    this.nativeStartServiceExtensionAbility(want, myCall);
  }

  startServiceExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined)=>void, reject:(err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      this.nativeStartServiceExtensionAbility(want, myCall);
    });
  }

  openLink(link: string, options?: OpenLinkOptions, callback?: AsyncCallback<AbilityResult, void>): Promise<void> {
    this.nativeOpenLinkCheck(link);
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      let callParam: AsyncCallbackWrapper<AbilityResult> | undefined = undefined;
      if (callback) {
        callParam = new AsyncCallbackWrapper<AbilityResult>(callback);
      }
      taskpool.execute(() => {
        this.nativeOpenLink(link, myCall, options, callParam);
      });
    });
  }

  isTerminating(): boolean {
    return this.nativeIsTerminating();
  }

  moveAbilityToBackground(): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute(() => {
        this.nativeMoveAbilityToBackground(syncCall);
      });
    });
  }

  requestModalUIExtension(pickerWant: Want, callback: AsyncCallback<void>): void {
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeRequestModalUIExtension(pickerWant, syncCall);
    });
  }

  requestModalUIExtension(pickerWant: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute(() => {
        this.nativeRequestModalUIExtension(pickerWant, syncCall);
      });
    });
  }

  requestModalUIExtensionWithAccount(pickerWant: Want, accountId: int): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute(() => {
        this.nativeRequestModalUIExtensionWithAccount(pickerWant, accountId, syncCall);
      });
    });
  }

  backToCallerAbilityWithResult(abilityResult: AbilityResult, requestCode: string): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute(() => {
        this.nativeBackToCallerAbilityWithResult(abilityResult, requestCode, syncCall);
      });
    });
  }

  setMissionLabel(label: string, callback: AsyncCallback<void>): void {
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    this.nativeSetMissionLabel(label, syncCall);
  }

  setMissionLabel(label: string): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      this.nativeSetMissionLabel(label, syncCall);
    });
  }

  setMissionContinueState(state: AbilityConstant.ContinueState): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeSetMissionContinueState(state, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  connectServiceExtensionAbility(want: Want, options: ConnectOptions): long {
    return this.nativeConnectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbility(connection: long, callback: AsyncCallback<void>): void {
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeDisconnectServiceExtensionAbility(connection, syncCall);
    });
  }

  disconnectServiceExtensionAbility(connection: long): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeDisconnectServiceExtensionAbility(connection, callback);
      })
    });
  }

  setColorMode(colorMode: ConfigurationConstant.ColorMode): void {
    this.nativeSetColorMode(colorMode);
  }

  startAbilityByType(type: string, wantParam: Record<string, RecordData>, startCallback: AbilityStartCallback,
    callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): BusinessError => {
      return this.nativeStartAbilityByTypeSync(type, wantParam, startCallback);
    });
    p1.then((e: Any) => {
      let retError = e as BusinessError;
      if (retError.code === 0) {
        callback(null, undefined);
      } else {
        callback(retError, undefined);
      }
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  showAbility(): Promise<void> {
  let p: Promise<void> =
    new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
    taskpool.execute((): void => {
      this.nativeChangeAbilityVisibility(true, myCall);
    });
  });
  return p;
}

hideAbility(): Promise<void> {
  let p: Promise<void> =
    new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
    taskpool.execute((): void => {
      this.nativeChangeAbilityVisibility(false, myCall);
    });
  });
  return p;
}

  startAbilityByType(type: string, wantParam: Record<string, RecordData>,
    startCallback: AbilityStartCallback): Promise<void> {
    let p = new Promise<void>((resolve: (v: undefined) => void, reject: (error: BusinessError) => void): void => {
      let p1 = taskpool.execute((): BusinessError => {
        return this.nativeStartAbilityByTypeSync(type, wantParam, startCallback);
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

  setRestoreEnabled(enabled: boolean): void {
    return this.nativeOnSetRestoreEnabled(enabled);
  }

  openAtomicService(appId: string, options?: AtomicServiceOptions): Promise<AbilityResult> {
    this.nativeOpenAtomicServiceCheck();
    return new Promise<AbilityResult>((resolve: (data: AbilityResult) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<AbilityResult>((err: BusinessError | null, data: AbilityResult | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as AbilityResult);
        } else {
          reject(err);
        }
      });
      taskpool.execute(() => {
        this.nativeOpenAtomicService(appId, asyncCall, options);
      });
    });
  }

  connectUIServiceExtensionAbility(want: Want, callback: UIServiceExtensionConnectCallback) : Promise<UIServiceProxy> {
    this.nativeWantCheck(want);
    return new Promise<UIServiceProxy>((resolve: (data: UIServiceProxy) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<UIServiceProxy>((err: BusinessError | null, data: UIServiceProxy | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as UIServiceProxy);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeConnectUIServiceExtensionAbility(want, callback, myCall);
      });
    });
  }

  startUIServiceExtensionAbility(want: Want): Promise<void> {
    this.nativeWantCheck(want);
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartUIServiceExtensionAbility(want, myCall);
      });
    });
  }

  disconnectUIServiceExtensionAbility(proxy: UIServiceProxy): Promise<void> {
    this.nativeDisconnectUIServiceExtensionCheck(proxy);
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeDisconnectUIServiceExtensionAbility(proxy, myCall);
      });
    });
  }

   requestDialogService(want: Want, result: AsyncCallback<dialogRequest.RequestResult>): void {
    let syncCall = new AsyncCallbackWrapper<dialogRequest.RequestResult>(result);
    taskpool.execute((): void => {
      this.nativeRequestDialogService(want, syncCall);
    }).catch((err: Error): void => {
      result(err as BusinessError, undefined);
    });
  }

  requestDialogService(want: Want): Promise<dialogRequest.RequestResult> {
    return new Promise<dialogRequest.RequestResult>((resolve: (data: dialogRequest.RequestResult) => void,
      reject: (err: BusinessError<void>) => void): void => {
      let syncCall = new AsyncCallbackWrapper<dialogRequest.RequestResult>((err: BusinessError | null,
        data: dialogRequest.RequestResult | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as dialogRequest.RequestResult);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeRequestDialogService(want, syncCall);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startSelfUIAbilityInCurrentProcess(want: Want, specifiedFlag: string, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (options == null) {
          this.nativeStartSelfUIAbilityInCurrentProcessSync(want, specifiedFlag, myCall);
        } else {
          this.nativeStartSelfUIAbilityInCurrentProcessSync(want, specifiedFlag, options, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  setAbilityInstanceInfo(label: string, icon: image.PixelMap): Promise<void> {
    this.nativeSetAbilityInstanceInfoCheck(label, icon);
    let p:Promise<void> = new Promise<void>((resolve: (data:undefined)=>void, reject: (err: BusinessError)=>void):void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null)=>{
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeSetAbilityInstanceInfo(label, icon, syncCall);
      });
    });
    return p;
  }

  connectServiceExtensionAbilityWithAccount(want: Want, accountId: int, options: ConnectOptions): long {
    return this.nativeConnectServiceExtensionAbilityWithAccount(want, accountId, options);
  }

  stopServiceExtensionAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallback<void>): void {
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStopServiceExtensionAbilityWithAccount(want, accountId, syncCall);
    });
  }

  stopServiceExtensionAbilityWithAccount(want: Want, accountId: int): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStopServiceExtensionAbilityWithAccount(want, accountId, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  stopServiceExtensionAbility(want: Want, callback: AsyncCallback<void>): void {
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStopServiceExtensionAbility(want, syncCall);
    });
  }

  stopServiceExtensionAbility(want: Want): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStopServiceExtensionAbility(want, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  startServiceExtensionAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallback<void>): void {
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartServiceExtensionAbilityWithAccount(want, accountId, syncCall);
    });
  }

  startServiceExtensionAbilityWithAccount(want: Want, accountId: int): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartServiceExtensionAbilityWithAccount(want, accountId, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  setMissionIcon(icon: image.PixelMap, callback: AsyncCallback<void>): void {
    this.nativeSetMissionIconCheck(icon);
    let syncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeSetMissionIcon(icon, syncCall);
    });
  }

  setMissionIcon(icon: image.PixelMap): Promise<void> {
    this.nativeSetMissionIconCheck(icon);
    let p = new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeSetMissionIcon(icon, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  revokeDelegator(): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeRevokeDelegator(asyncCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
    return p;
  }

  connectAppServiceExtensionAbility(want: Want, options: ConnectOptions): long {
    return this.nativeConnectAppServiceExtensionAbility(want, options);
  }

  disconnectAppServiceExtensionAbility(connection: long): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeDisconnectAppServiceExtensionAbility(connection, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startAppServiceExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined)=>void, reject:(err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartAppServiceExtensionAbility(want, asyncCall);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  stopAppServiceExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStopAppServiceExtensionAbility(want, asyncCall);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startAbilityForResultWithAccount(want: Want, accountId: int, callback: AsyncCallback<AbilityResult, void>): void {
    this.nativeStartAbilityForResultWithAccountCheck();
    let myCall = new AsyncCallbackWrapper<AbilityResult>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityForResultWithAccount(want, accountId, myCall);
    });
  }

  startAbilityForResultWithAccount(want: Want, accountId: int, options: StartOptions,
    callback: AsyncCallback<void>): void {
    this.nativeStartAbilityForResultWithAccountCheck();
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityForResultWithAccountVoid(want, accountId, options, myCall);
    });
  }

  startAbilityForResultWithAccount(want: Want, accountId: int, options?: StartOptions): Promise<AbilityResult> {
    this.nativeStartAbilityForResultWithAccountCheck();
    let p = new Promise<AbilityResult>((resolve: (data: AbilityResult) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall =
        new AsyncCallbackWrapper<AbilityResult>((err: BusinessError | null, data: AbilityResult | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as AbilityResult);
          } else {
            reject(err);
          }
        });
      taskpool.execute((): void => {
        if (options == null) {
          this.nativeStartAbilityForResultWithAccount(want, accountId, myCall);
        } else {
          this.nativeStartAbilityForResultWithAccountResult(want, accountId, options as StartOptions, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  startAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityWithAccountSync(want, accountId, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityWithAccount(want: Want, accountId: int, options: StartOptions, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityWithAccountSync(want, accountId, options, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityWithAccount(want: Want, accountId: int, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (options == null) {
          this.nativeStartAbilityWithAccountSync(want, accountId, asyncCall);
        } else {
          this.nativeStartAbilityWithAccountSync(want, accountId, options as StartOptions, asyncCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  restoreWindowStage(localStorage: LocalStorage): void {
    this.nativeRestoreWindowStage(localStorage);
  }

  startAbilityAsCaller(want: Want, callback: AsyncCallback<void>): void {
    let asyncCallback = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityAsCaller(want, asyncCallback, undefined);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityAsCaller(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let asyncCallback = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityAsCaller(want, asyncCallback, options);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityAsCaller(want: Want, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCallback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartAbilityAsCaller(want, asyncCallback, options);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startRecentAbility(want: Want, callback: AsyncCallback<void>): void {
    let asyncCallback = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartRecentAbility(want, asyncCallback, undefined);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startRecentAbility(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let asyncCallback = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartRecentAbility(want, asyncCallback, options);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startRecentAbility(want: Want, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCallback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartRecentAbility(want, asyncCallback, options);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  restartApp(want: Want): Promise<void> {
    return taskpool.execute((): void => {
      this.nativeRestartAppSync(want);
    }) as Promise<void>;
  }

  setMissionWindowIcon(windowIcon: image.PixelMap): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let syncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute(() => {
        this.nativeSetMissionWindowIcon(windowIcon, syncCall);
      });
    });
  }

  setOnNewWantSkipScenarios(scenarios: int): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let callback = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeSetOnNewWantSkipScenarios(scenarios, callback);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }
}
