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

import { AbilityResult } from 'ability.abilityResult';
import { AsyncCallback } from '@ohos.base';
import { BusinessError, RecordData } from '@ohos.base';
import uiExtensionHost from '@ohos.uiExtensionHost';
import { LocalStorage } from '@ohos.arkui.stateManagement';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import AbilityStartCallback from 'application.AbilityStartCallback';
import StartOptions from '@ohos.app.ability.StartOptions';
import uiExtension from '@ohos.arkui.uiExtension';
import StartOptions from '@ohos.app.ability.StartOptions';
import Want from '@ohos.app.ability.Want';

export type OnReceiveDataCallback = (data: Record<string, RecordData>) => void;
export type OnReceiveDataForResultCallback = (data: Record<string, RecordData>) => Record<string, RecordData>

class Cleaner {
  static callback(cleaner: Cleaner): void {
      cleaner.clean()
  }
  constructor(targetPtr: long) {
      this.targetPtr = targetPtr
  }
  native clean(): void
  private targetPtr: long = 0
}
let destroyRegister = new FinalizationRegistry<Cleaner>(Cleaner.callback)
let unregisterToken = new object()

export default class UIExtensionContentSession {
  static {
    loadLibrary("ui_extension_content_session_ani_kit.z");
  }
  nativeContextSession:long = 0;
  private cleaner: Cleaner | null = null;
  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr)
      destroyRegister.register(this, this.cleaner!, unregisterToken);
  }
  unregisterCleaner(): void {
      destroyRegister.unregister(unregisterToken);
  }
  constructor(contextSession:long) {
    if(this.nativeContextSession == 0){
          this.nativeContextSession = contextSession;
    }
    this.registerCleaner(this.nativeContextSession)
  }
  private static native nativeTransferStatic(input: ESValue): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;

  static transferStatic(input: Any): Object {
    return UIExtensionContentSession.nativeTransferStatic(ESValue.wrap(input));
  }

  static transferDynamic(input: Object): Any {
    return UIExtensionContentSession.nativeTransferDynamic(input).unwrap();
  }
  native nativeSendData(data: Record<string, RecordData>): void;
  native loadContent(path: string, storage?: LocalStorage): void;
  native loadContentByName(path: string, storage?: LocalStorage): void;
  native setWindowBackgroundColor(color: string): void;
  native getUIExtensionHostWindowProxy(): uiExtensionHost.UIExtensionHostWindowProxy;
  native getUIExtensionWindowProxy(): uiExtension.WindowProxy;
  native nativeSetReceiveDataCallback(callback: OnReceiveDataCallback): void;
  native nativeSetReceiveDataForResultCallback(callback: OnReceiveDataForResultCallback): void;
  native terminateSelfSync(callback:AsyncCallbackWrapper<void>): void;
  native terminateSelfWithResultSync(parameter: AbilityResult, callback:AsyncCallbackWrapper<void>): void;
  private native nativeStartAbilityByType(type: string, wantParam: Record<string, RecordData>,
    startCallback: AbilityStartCallback, callback: AsyncCallbackWrapper<void>): void;
  native nativeSetWindowPrivacyMode(isPrivacyMode: boolean, callback:AsyncCallbackWrapper<void>): void;
  native nativeStartAbilitySync(want: Want, callback: AsyncCallbackWrapper<void>): void;
  native nativeStartAbilitySync(want: Want, options: StartOptions, callback:AsyncCallbackWrapper<void>): void;
  native nativeStartAbilityForResult(want: Want, callback:AsyncCallbackWrapper<AbilityResult>): void;
  native nativeStartAbilityForResult(want: Want, startOptions: StartOptions, callback:AsyncCallbackWrapper<AbilityResult>): void;
  native nativeStartAbilityAsCaller(want: Want, callback: AsyncCallbackWrapper<void>, options?: StartOptions): void;

  sendData(data: Record<string, RecordData>): void {
    this.nativeSendData(data);
  }

  setReceiveDataCallback(callback: OnReceiveDataCallback): void {
    this.nativeSetReceiveDataCallback(callback);
  }

  setReceiveDataForResultCallback(callback: OnReceiveDataForResultCallback): void {
    this.nativeSetReceiveDataForResultCallback(callback);
  }

  terminateSelf(callback:AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.terminateSelfSync(myCall);
    });
  }

  terminateSelf(): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.terminateSelfSync(myCall);
      });
    });
    return p;
  }

  terminateSelfWithResult(parameter: AbilityResult, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.terminateSelfWithResultSync(parameter, myCall);
    });
  }

  terminateSelfWithResult(parameter: AbilityResult): Promise<void> {
    let p = new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.terminateSelfWithResultSync(parameter, myCall);
      });
    });
    return p;
  }

  startAbilityByType(type: string, wantParam: Record<string, RecordData>, startCallback: AbilityStartCallback,
    callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError<void> | null) => {
      if (err == null || err.code == 0) {
        callback(null, undefined);
      } else {
        callback(err, undefined);
      }
    });
    taskpool.execute((): void => {
      this.nativeStartAbilityByType(type, wantParam, startCallback, asyncCall);
    }).then(() => {}, (err: Error): void => {
      asyncCall(err as BusinessError);
    });
  }

  startAbilityByType(type: string, wantParam: Record<string, RecordData>,
    startCallback: AbilityStartCallback): Promise<void> {
    let p = new Promise<void>((resolve: (v: undefined) => void, reject: (error: BusinessError<void>) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError<void> | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartAbilityByType(type, wantParam, startCallback, myCall);
      }).then(() => {}, (err: Error): void => {
        myCall(err as BusinessError);
      });
    });
    return p;
  }

  setWindowPrivacyMode(isPrivacyMode: boolean): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeSetWindowPrivacyMode(isPrivacyMode, asyncCall);
      });
    });
  }

  setWindowPrivacyMode(isPrivacyMode: boolean, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeSetWindowPrivacyMode(isPrivacyMode, asyncCall);
    });
  }

   startAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilitySync(want, myCall);
    });
  }

  startAbility(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilitySync(want, options, myCall);
    });
  }

  startAbility(want: Want, options?: StartOptions): Promise<void> {
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
      callback(err  as BusinessError, undefined);
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
}
