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

import AbilityLifecycleCallback from '@ohos.app.ability.AbilityLifecycleCallback';
import InteropAbilityLifecycleCallback from '../@ohos.app.ability.InteropAbilityLifecycleCallback';
import Context from 'application.Context'
import { BusinessError, AsyncCallback } from '@ohos.base'
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import Want from '@ohos.app.ability.Want';
import window from '@ohos.window';
import { ProcessInformation } from 'application.ProcessInformation';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import EnvironmentCallback from '@ohos.app.ability.EnvironmentCallback';
import type ApplicationStateChangeCallback from '@ohos.app.ability.ApplicationStateChangeCallback';
import { AbilityUtils } from '../utils/AbilityUtils';
import systemConfiguration from '../@ohos.app.ability.systemConfiguration';
import UIAbility from '@ohos.app.ability.UIAbility';

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

export default class ApplicationContext extends Context {
  static {
    loadLibraryWithPermissionCheck("application_context_ani_kit.z", "application.ApplicationContext");
  }

  etsApplicationContextPtr: long = 0;
  private cleaner: Cleaner | null = null;

  constructor(ptr: long) {
    if (this.etsApplicationContextPtr == 0) {
      this.etsApplicationContextPtr = ptr;
    }
    this.registerCleaner(this.etsApplicationContextPtr);
  }

  registerCleaner(ptr: long): void {
    this.cleaner = new Cleaner(ptr);
    destroyRegister.register(this, this.cleaner!, unregisterToken);
  }

  unregisterCleaner(): void {
    destroyRegister.unregister(unregisterToken);
  }

  public native nativeOnLifecycleCallbackSync(type: string, callback: AbilityLifecycleCallback): int;
  public native nativeOffLifecycleCallbackSync(type: string, callbackId: int,
    callback: AsyncCallbackWrapper<void>): void;
  public native nativeOffAbilityLifecycleCheck(): void;
  public native nativekillAllProcessesSync(clearPageStack: boolean, callback: AsyncCallbackWrapper<void>): void;
  public native nativepreloadUIExtensionAbilitySync(want: Want, callback: AsyncCallbackWrapper<void>): void;
  public native setSupportedProcessCacheSync(isSupported : boolean):void;
  public native nativegetRunningProcessInformation(callback: AsyncCallbackWrapper<Array<ProcessInformation>>): void;
  public native nativeclearUpApplicationData(callback: AsyncCallbackWrapper<void>): void;
  public native nativesetLanguage(language: string): void;
  public native nativesetFontSizeScale(fontSizeScale: double): void;
  public native nativesetColorMode(colorMode: ConfigurationConstant.ColorMode): void;
  public native nativesetFont(font: string): void;
  public native nativerestartApp(want: Want): void;
  public native nativeGetAllRunningInstanceKeys(callback: AsyncCallbackWrapper<Array<string>>): void;
  public native nativeGetAllWindowStages(callback: AsyncCallbackWrapper<Array<window.WindowStage>>): void;
  public native nativeOnEnvironmentSync(callback: EnvironmentCallback): int;
  public native nativeOffEnvironmentSync(callbackId: int, callback: AsyncCallbackWrapper<void>): void;
  public native nativeOffEnvironmentCheck(): void;
  public native nativeOnApplicationStateChangeSync(callback: ApplicationStateChangeCallback): void;
  public native nativeOnSystemConfigurationUpdatedSync(callback: systemConfiguration.UpdatedCallback): void;
  public native nativeOffSystemConfigurationUpdatedSync(callback: systemConfiguration.UpdatedCallback): void;
  public native nativeOffApplicationStateChangeSync(callback?: ApplicationStateChangeCallback): void;
  public native nativegetCurrentAppCloneIndex(): int;
  public native nativegetCurrentInstanceKey(): string;
  public native nativeEnableDelayedProcessExitSync(callback: AsyncCallbackWrapper<void>): void;
  public native nativeDisableDelayedProcessExitSync(callback: AsyncCallbackWrapper<void>): void;
  public native nativeStartSelfUIAbilitySync(want: Want, callback: AsyncCallbackWrapper<void>): void;
  private native nativeGetUIAbilityByInstanceId(instanceId: string): UIAbility;
  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'ApplicationContext';

  setSupportedProcessCache(isSupported : boolean):void {
    this.setSupportedProcessCacheSync(isSupported);
  }
  preloadUIExtensionAbility(want: Want): Promise<void> {
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
          this.nativepreloadUIExtensionAbilitySync(want, myCall);
        });
      });
    return p;
  }

  killAllProcesses(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativekillAllProcessesSync(false, myCall);
    });
  }

  killAllProcesses(): Promise<void> {
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
          this.nativekillAllProcessesSync(false, myCall);
        });
      });
    return p;
  }

  killAllProcesses(clearPageStack: boolean): Promise<void> {
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
          this.nativekillAllProcessesSync(clearPageStack, myCall);
        });
      });
    return p;
  }

  getRunningProcessInformation(callback: AsyncCallback<Array<ProcessInformation>>): void {
    let myCall = new AsyncCallbackWrapper<Array<ProcessInformation>>(callback);
    taskpool.execute((): void => {
      this.nativegetRunningProcessInformation(myCall);
    });
  }

  getRunningProcessInformation(): Promise<Array<ProcessInformation>> {
    let p = new Promise<Array<ProcessInformation>>((resolve: (data: Array<ProcessInformation>) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<Array<ProcessInformation>>((err: BusinessError | null,
        data: Array<ProcessInformation> | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Array<ProcessInformation>);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativegetRunningProcessInformation(myCall);
      });
    });
    return p;
  }

  clearUpApplicationData(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeclearUpApplicationData(myCall);
    });
  }

  clearUpApplicationData(): Promise<void> {
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
          this.nativeclearUpApplicationData(myCall);
        });
      });
    return p;
  }

  setLanguage(language: string): void {
    this.nativesetLanguage(language);
  }

  setFontSizeScale(fontSizeScale: double): void {
    this.nativesetFontSizeScale(fontSizeScale);
  }

  setColorMode(colorMode: ConfigurationConstant.ColorMode): void {
    this.nativesetColorMode(colorMode);
  }

  setFont(font: string): void {
    this.nativesetFont(font);
  }

  restartApp(want: Want): void {
    this.nativerestartApp(want);
  }

  enableDelayedProcessExit(): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCbWrapper = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeEnableDelayedProcessExitSync(asyncCbWrapper);
        });
      });
    return p;
  }

  disableDelayedProcessExit(): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCbWrapper = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeDisableDelayedProcessExitSync(asyncCbWrapper);
        });
      });
    return p;
  }

  startSelfUIAbility(want: Want): Promise<void> {
    let p =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let asyncCbWrapper = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartSelfUIAbilitySync(want, asyncCbWrapper);
        });
      });
    return p;
  }

  getAllRunningInstanceKeys(): Promise<Array<string>> {
    let p = new Promise<Array<string>>((resolve: (data: Array<string>) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<Array<string>>((err: BusinessError | null,
        data: Array<string> | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Array<string>);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeGetAllRunningInstanceKeys(myCall);
      });
    });
    return p;
  }

  getAllWindowStages(): Promise<Array<window.WindowStage>> {
    let p = new Promise<Array<window.WindowStage>>((resolve: (data: Array<window.WindowStage>) => void,
      reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<Array<window.WindowStage>>((err: BusinessError | null,
        data: Array<window.WindowStage> | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Array<window.WindowStage>);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeGetAllWindowStages(myCall);
      });
    });
    return p;
  }

  onEnvironment(callback: EnvironmentCallback): int {
    return this.nativeOnEnvironmentSync(callback);
  }

  offEnvironment(callbackId: int, callback: AsyncCallback<void>): void {
    this.nativeOffEnvironmentCheck();
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeOffEnvironmentSync(callbackId, myCall);
    });
  }

  offAbilityLifecycle(callbackId: int, callback: AsyncCallback<void>): void {
    this.nativeOffAbilityLifecycleCheck();
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeOffLifecycleCallbackSync('abilityLifecycle', callbackId, myCall);
    });
  }

  offEnvironment(callbackId: int): Promise<void> {
    this.nativeOffEnvironmentCheck();
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
          this.nativeOffEnvironmentSync(callbackId, myCall);
        });
      });
    return p;
  }

  offAbilityLifecycle(callbackId: int): Promise<void> {
    this.nativeOffAbilityLifecycleCheck();
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
          this.nativeOffLifecycleCallbackSync('abilityLifecycle', callbackId, myCall);
        });
      });
    return p;
  }

  onApplicationStateChange(callback: ApplicationStateChangeCallback): void {
    this.nativeOnApplicationStateChangeSync(callback);
  }

  onSystemConfigurationUpdated(callback: systemConfiguration.UpdatedCallback): void {
    this.nativeOnSystemConfigurationUpdatedSync(callback);
  }

  offSystemConfigurationUpdated(callback: systemConfiguration.UpdatedCallback): void {
    this.nativeOffSystemConfigurationUpdatedSync(callback);
  }

  offApplicationStateChange(callback?: ApplicationStateChangeCallback): void {
    this.nativeOffApplicationStateChangeSync(callback);
  }

  getCurrentInstanceKey(): string {
    return this.nativegetCurrentInstanceKey();
  }

  getCurrentAppCloneIndex(): int {
    return this.nativegetCurrentAppCloneIndex();
  }

  getUIAbilityByInstanceId(instanceId: string): UIAbility {
    return this.nativeGetUIAbilityByInstanceId(instanceId);
  }

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    console.log('transferStatic contextType: ' + type);
    return ApplicationContext.nativeTransferStatic(ESValue.wrap(input), type);
  }

  static transferDynamic(input: Object): Any {
    return ApplicationContext.nativeTransferDynamic(input).unwrap();
  }

  onAbilityLifecycle(callback: AbilityLifecycleCallback): int {
    return this.nativeOnLifecycleCallbackSync('abilityLifecycle', callback);
  }

  public native nativeOnInteropLifecycleCallbackSync(callback: InteropAbilityLifecycleCallback): void;
  public native nativeOffInteropLifecycleCallbackSync(callback?: InteropAbilityLifecycleCallback): void;
  onInteropAbilityLifecycle(callback: InteropAbilityLifecycleCallback): void {
    this.nativeOnInteropLifecycleCallbackSync(callback);
  }
  offInteropAbilityLifecycle(callback?: InteropAbilityLifecycleCallback): void {
    this.nativeOffInteropLifecycleCallbackSync(callback);
  }
}
