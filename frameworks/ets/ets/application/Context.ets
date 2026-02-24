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

import BaseContext from 'application.BaseContext'
import EventHub from 'application.EventHub';
import ApplicationContext from 'application.ApplicationContext'
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import { AsyncCallback } from '@ohos.base';
import { BusinessError } from '@ohos.base';
import contextConstant from '@ohos.app.ability.contextConstant'
import { ApplicationInfo } from 'bundleManager.ApplicationInfo'
import resmgr from '@ohos.resourceManager'

class Cleaner {
  public ptr: long = 0;
  public nativePtr: long = 0;
  constructor(ptr: long, nativePtr: long) {
    this.ptr = ptr;
    this.nativePtr = nativePtr;
  }
  native clean(): void;
}
export function callback(cleaner: Cleaner): void {
  cleaner.clean();
}
let destroyRegister = new FinalizationRegistry<Cleaner>(callback);
let unregisterToken = new object();

export default class Context extends BaseContext {
  static {
    loadLibraryWithPermissionCheck("context_ani_kit.z", "application.Context");
  }
  nativePtr: long = 0;
  
  etsContextPtr: long = 0;
  private cleaner: Cleaner | null = null;
  filesDir: string = "";
  tempDir: string = "";
  preferencesDir: string = "";
  databaseDir: string = "";
  cacheDir: string = "";
  cloudFileDir: string = "";
  distributedFilesDir: string = "";
  bundleCodeDir: string = "";
  resourceDir: string = "";
  applicationInfo!: ApplicationInfo;
  resourceManager!: resmgr.ResourceManager;
  eventHub: EventHub = new EventHub();
  processName: string = "";
  constructor() {
    if (this.eventHub == null) {
      this.eventHub = new EventHub();
    }
  }

  constructor(ptr: long) {
    if (this.eventHub == null) {
      this.eventHub = new EventHub();
    }
    this.nativePtr = ptr;
  }

  private setEtsContextPtr(ptr: long) {
    if (this.etsContextPtr == 0) {
      this.etsContextPtr = ptr;
    }
    this.registerCleaner(this.etsContextPtr, this.nativePtr);
  }
  registerCleaner(ptr: long, nativePtr: long): void {
    this.cleaner = new Cleaner(ptr, nativePtr);
    destroyRegister.register(this, this.cleaner!, unregisterToken);
  }
  unregisterCleaner(): void {
    destroyRegister.unregister(unregisterToken);
  }

  public native getApplicationContextSync(): ApplicationContext;
  public native createModuleResourceManagerSync(bundleName: string, moduleName: string): resmgr.ResourceManager;
  private native switchArea(mode: contextConstant.AreaMode): void;
  private native getArea(): contextConstant.AreaMode;
  private native getLogFileDir(): string;
  public native nativeGetGroupDir(dataGroupID: string, callback: AsyncCallbackWrapper<string>): void;
  public native nativeCreateDisplayContext(displayId: long): Context;
  public native nativeCreateAreaModeContext(areaMode: contextConstant.AreaMode): Context;
  public native nativeCreateSystemHspModuleResourceManager(bundleName: string, moduleName: string): resmgr.ResourceManager;
  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'Context';

  getApplicationContext(): ApplicationContext {
    return this.getApplicationContextSync();
  }

  createModuleResourceManager(bundleName: string, moduleName: string): resmgr.ResourceManager {
    return this.createModuleResourceManagerSync(bundleName, moduleName);
  }

  set area(mode: contextConstant.AreaMode) {
    this.switchArea(mode);
  }

  get area(): contextConstant.AreaMode {
    return this.getArea();
  }

  get logFileDir(): string {
    return this.getLogFileDir();
  }

  getGroupDir(dataGroupID: string, callback: AsyncCallback<string>): void {
    let myCall = new AsyncCallbackWrapper<string>(callback);
    taskpool.execute((): void => {
      this.nativeGetGroupDir(dataGroupID, myCall);
    });
  }

  getGroupDir(dataGroupID: string): Promise<string> {
    return new Promise<string>((resolve: (data: string) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<string>((err: BusinessError | null, data: string | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as string);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeGetGroupDir(dataGroupID, myCall);
      });
    });
  }

  createDisplayContext(displayId: long): Context {
    return this.nativeCreateDisplayContext(displayId);
  }

  createAreaModeContext(areaMode: contextConstant.AreaMode): Context {
    return this.nativeCreateAreaModeContext(areaMode);
  }

  createSystemHspModuleResourceManager(bundleName: string, moduleName: string): resmgr.ResourceManager {
    return this.nativeCreateSystemHspModuleResourceManager(bundleName, moduleName);
  }

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    console.log('transferStatic type: ' + type);
    return Context.nativeTransferStatic(ESValue.wrap(input), type);
  }

  static transferDynamic(input: Object): Any {
    return Context.nativeTransferDynamic(input).unwrap();
  }
}
