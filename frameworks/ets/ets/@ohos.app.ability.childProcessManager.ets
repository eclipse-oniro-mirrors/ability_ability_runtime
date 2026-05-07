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

import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { ChildProcessArgs } from '@ohos.app.ability.ChildProcessArgs';
import { ChildProcessOptions } from '@ohos.app.ability.ChildProcessOptions';

namespace childProcessManager {
  loadLibraryWithPermissionCheck("ability_child_process_manager_ani_kit.z", "@ohos.app.ability.childProcessManager");

  export enum StartMode {
    SELF_FORK = 0,
    APP_SPAWN_FORK = 1,
  }

  export native function nativeStartChildProcess(srcEntry: string, startMode: StartMode, callback: AsyncCallbackWrapper<int>): void;
  export native function nativeStartArkChildProcess(srcEntry: string, args: ChildProcessArgs, callback: AsyncCallbackWrapper<int>): void;
  export native function nativeStartArkChildProcess(srcEntry: string, args: ChildProcessArgs, options: ChildProcessOptions, callback: AsyncCallbackWrapper<int>): void;
  export native function nativeStartNativeChildProcess(entryPoint: string, args: ChildProcessArgs, callback: AsyncCallbackWrapper<int>): void;
  export native function nativeStartNativeChildProcess(entryPoint: string, args: ChildProcessArgs, options: ChildProcessOptions, callback: AsyncCallbackWrapper<int>): void;
  export native function isArkChildProcessSupported(): boolean;
  export native function isNativeChildProcessSupported(): boolean;

  export native function nativeStartChildProcessCheck(srcEntry: string, startMode: StartMode): void;
  export native function nativeStartArkChildProcessCheck(srcEntry: string, args: ChildProcessArgs, options?: ChildProcessOptions): void;
  export native function nativeStartNativeChildProcessCheck(entryPoint: string, args: ChildProcessArgs, options?: ChildProcessOptions): void;

  export function startChildProcess(srcEntry: string, startMode: StartMode): Promise<int> {
    childProcessManager.nativeStartChildProcessCheck(srcEntry, startMode);
    let p = new Promise<int>((resolve: (data: int) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<int>((err: BusinessError | null, data: int | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as int);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        childProcessManager.nativeStartChildProcess(srcEntry, startMode, myCall);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  export function startChildProcess(srcEntry: string, startMode: StartMode, callback: AsyncCallback<int>): void {
    childProcessManager.nativeStartChildProcessCheck(srcEntry, startMode);
    let myCall = new AsyncCallbackWrapper<int>(callback);
    taskpool.execute((): void => {
      childProcessManager.nativeStartChildProcess(srcEntry, startMode, myCall);
    });
  }

  export function startArkChildProcess(srcEntry: string, args: ChildProcessArgs,
    options?: ChildProcessOptions): Promise<int> {
    childProcessManager.nativeStartArkChildProcessCheck(srcEntry, args, options);
    let p = new Promise<int>((resolve: (data: int) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<int>((err: BusinessError | null, data: int | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as int);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (options == null) {
          childProcessManager.nativeStartArkChildProcess(srcEntry, args, myCall);
        } else {
          childProcessManager.nativeStartArkChildProcess(srcEntry, args, options as ChildProcessOptions, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }

  export function startNativeChildProcess(entryPoint: string, args: ChildProcessArgs,
    options?: ChildProcessOptions): Promise<int> {
    childProcessManager.nativeStartNativeChildProcessCheck(entryPoint, args, options);
    let p = new Promise<int>((resolve: (data: int) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<int>((err: BusinessError | null, data: int | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as int);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (options == null) {
          childProcessManager.nativeStartNativeChildProcess(entryPoint, args, myCall);
        } else {
          childProcessManager.nativeStartNativeChildProcess(entryPoint, args, options as ChildProcessOptions, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
    return p;
  }
}

export default childProcessManager;