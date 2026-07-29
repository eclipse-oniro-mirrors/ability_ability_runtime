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

import Context from 'application.Context';
import { BusinessError, AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import ApplicationContext from 'application.ApplicationContext';

export default namespace application {
  loadLibraryWithPermissionCheck("application_ani.z", "@ohos.app.ability.application")

  export enum AppPreloadType {
    UNSPECIFIED = 0,
    TYPE_CREATE_PROCESS = 1,
    TYPE_CREATE_ABILITY_STAGE = 2,
    TYPE_CREATE_WINDOW_STAGE = 3,
    TYPE_CREATE_BACKGROUND_ABILITY = 4,
  };

  export native function nativeCreateModuleContext(context: Context, bundleName: string, moduleName: string,
    callback: AsyncCallbackWrapper<Context>): void;

  export native function nativeCreateModuleContextSync(context: Context, moduleName: string): Context;

  export native function nativeCreateBundleContext(context: Context, bundleName: string,
    callback: AsyncCallbackWrapper<Context>): void;

  export native function nativeCreatePluginModuleContext(context: Context, pluginBundleName: string,
    pluginModuleName: string, callback: AsyncCallbackWrapper<Context>): void;

  export native function nativeGetApplicationContext(): ApplicationContext;

  export native function nativeDemoteCurrentFromCandidateMasterProcess(callback: AsyncCallbackWrapper<void>): void;

  export native function nativePromoteCurrentToCandidateMasterProcess(isInsertToHead: boolean,
    callback:AsyncCallbackWrapper<void>): void;

  export native function nativeExitMasterProcessRole(callback: AsyncCallbackWrapper<void>): void;

  export native function nativeCreatePluginModuleContextForHostBundle(context: Context, pluginBundleName: string,
    pluginModuleName: string, hostBundleName: string, callback: AsyncCallbackWrapper<Context>): void;

  export native function nativeCreatePluginModuleContextForHostBundleCheck(context: Context, pluginBundleName: string,
    pluginModuleName: string, hostBundleName: string): void;

  export native function nativeGetApplicationContextInstance(): ApplicationContext;

  export native function nativeGetAppPreloadType(): AppPreloadType;

  export native function nativeCreateModuleContextCheck(context: Context, moduleName: string, bundleName?: string): void;
  export function createModuleContext(context: Context, moduleName: string): Promise<Context> {
    application.nativeCreateModuleContextCheck(context, moduleName);
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreateModuleContext(context, "", moduleName, myCall);
        });
      });
    return p;
  }

  export function createModuleContext(context: Context, bundleName: string, moduleName: string): Promise<Context> {
    application.nativeCreateModuleContextCheck(context, moduleName, bundleName);
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreateModuleContext(context, bundleName, moduleName, myCall);
        });
      });
    return p;
  }

  export function createModuleContextSync(context: Context, moduleName: string): Context {
    return application.nativeCreateModuleContextSync(context, moduleName);
  }

  export native function nativeCreateBundleContextCheck(context: Context, bundleName: string): void;
  export function createBundleContext(context: Context, bundleName: string): Promise<Context> {
    application.nativeCreateBundleContextCheck(context, bundleName);
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreateBundleContext(context, bundleName, myCall);
        });
      });
    return p;
  }

  export native function nativeCreatePluginModuleContextCheck(context: Context, pluginBundleName: string,
    pluginModuleName: string): void;
  export function createPluginModuleContext(context: Context, pluginBundleName: string,
    pluginModuleName: string): Promise<Context> {
    application.nativeCreatePluginModuleContextCheck(context, pluginBundleName, pluginModuleName);
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreatePluginModuleContext(context, pluginBundleName, pluginModuleName, myCall);
        });
      });
    return p;
  }

  export function demoteCurrentFromCandidateMasterProcess(): Promise<void> {
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
          application.nativeDemoteCurrentFromCandidateMasterProcess(myCall);
        });
      });
    return p;
  }

  export function promoteCurrentToCandidateMasterProcess(isInsertToHead: boolean): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject:(err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        application.nativePromoteCurrentToCandidateMasterProcess(isInsertToHead, asyncCall);
      });
    });
  }

  export function getApplicationContext(): ApplicationContext {
    return nativeGetApplicationContext();
  }

  export function getApplicationContextInstance(): ApplicationContext {
    return nativeGetApplicationContextInstance();
  }

  export function getAppPreloadType(): AppPreloadType {
    return nativeGetAppPreloadType();
  }

  export function exitMasterProcessRole(): Promise<void> {
    let p: Promise<void> =
      new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null)=> {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
            application.nativeExitMasterProcessRole(myCall);
        });
      });
    return p;
  }

  export function createPluginModuleContextForHostBundle(context: Context, pluginBundleName: string,
    pluginModuleName: string, hostBundleName: string): Promise<Context> {
    application.nativeCreatePluginModuleContextForHostBundleCheck(context, pluginBundleName, pluginModuleName, hostBundleName);
    let p =
      new Promise<Context>((resolve: (data: Context) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<Context>((err: BusinessError | null, data: Context | undefined) => {
          if (err == null || err.code == 0) {
            resolve(data as Context);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          application.nativeCreatePluginModuleContextForHostBundle(context, pluginBundleName, pluginModuleName, hostBundleName, myCall);
        });
      });
    return p;
  }
}