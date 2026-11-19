/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError, AsyncCallback, RecordData } from '@ohos.base';
import Want from '@ohos.app.ability.Want';
import { WantAgentInfo as _WantAgentInfo } from 'wantAgent.wantAgentInfo';
import { LocalWantAgentInfo as _LocalWantAgentInfo } from 'wantAgent.wantAgentInfo';
import { TriggerInfo as _TriggerInfo } from 'wantAgent.triggerInfo';
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { AbilityUtils } from './utils/AbilityUtils';
import Context from 'application.Context';

const ERROR_CODE_INVALID_PARAM: int = 401;


namespace wantAgent {
  loadLibraryWithPermissionCheck("aniwantagent.z", "@ohos.app.ability.wantAgent");

  class Cleaner {
    public ptr: long = 0;

    constructor(ptr: long) {
      this.ptr = ptr;
    }

    native clean(): void;
  }

  export function callback(cleaner: Cleaner): void {
    cleaner.clean();
  }

  let destroyRegister: FinalizationRegistry<Cleaner> = new FinalizationRegistry<Cleaner>(callback);
  let unregisterToken: object = new object();

  class WantAgentCls {
    wantAgentPtr: long = 0;
    private cleaner: Cleaner | null = null;

    constructor(ptr: long) {
      if (this.wantAgentPtr == 0) {
        this.wantAgentPtr = ptr;
      }
      this.registerCleaner(this.wantAgentPtr);
    }

    registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr);
      destroyRegister.register(this, this.cleaner!, unregisterToken);
    }

    unregisterCleaner(): void {
      destroyRegister.unregister(unregisterToken);
    }
  }

  native function nativeGetBundleName(agent: WantAgent, callback: AsyncCallbackWrapper<string>): void;
  native function nativeGetUid(agent: WantAgent, callback: AsyncCallbackWrapper<int>): void;
  native function nativeGetOperationType(agent: WantAgent, callback: AsyncCallbackWrapper<int>): void;
  native function nativeCancel(agent: WantAgent, callback: AsyncCallbackWrapper<void>): void;
  native function nativeEqual(agent: WantAgent, otherAgent: WantAgent, callback: AsyncCallbackWrapper<boolean>): void;
  native function nativeTrigger(agent: WantAgent, triggerInfo: TriggerInfo, callback?: AsyncCallbackWrapper<CompleteData>): void;
  native function nativeGetWant(agent: WantAgent, callback: AsyncCallbackWrapper<Want>): void;
  native function nativeGetWantAgent(agent: WantAgentInfo, callback: AsyncCallbackWrapper<WantAgent>): void;
  native function nativeIsLocalWantAgent(agent: WantAgent): boolean;
  native function nativeCreateLocalWantAgent(info: LocalWantAgentInfo): WantAgent;
  native function nativeTriggerCheck(agent: WantAgent, triggerInfo: TriggerInfo): void;
  native function nativeTriggerAsync(agent: WantAgent, triggerInfo: TriggerInfo, context: Context, callback: AsyncCallbackWrapper<CompleteData>): void;

  export function getBundleName(agent: WantAgent, callback: AsyncCallback<string>): void {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgent failed. Agent must be a WantAgent.');
    }
    let call = new AsyncCallbackWrapper<string>(callback);
    taskpool.execute((): void => {
      try {
        nativeGetBundleName(agent, call);
      } catch (err) {
        callback(err as BusinessError, "");
      }
    });
  }
  export native function nativetriggerAsyncCheck(agent: WantAgent, triggerInfo: TriggerInfo, context: Context): void;
  export function triggerAsync(agent: WantAgent, triggerInfo: TriggerInfo, context: Context): Promise<CompleteData> {
    nativetriggerAsyncCheck(agent, triggerInfo, context);
    return new Promise<CompleteData>((resolve: (data: CompleteData) => void,
      reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<CompleteData>((err: BusinessError | null, data: CompleteData | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as CompleteData);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeTriggerAsync(agent, triggerInfo, context, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function setWantAgentMultithreading(isMultithreadingSupported: boolean): void {
 
  }

  export function getBundleName(agent: WantAgent): Promise<string> {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgent failed. Agent must be a WantAgent.');
    }
    return new Promise<string>((resolve: (data: string) => void, reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<string>((err: BusinessError | null, data: string | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as string);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeGetBundleName(agent, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function getUid(agent: WantAgent, callback: AsyncCallback<int>): void {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgent error. Agent must be a WantAgent.');
    }
    let call = new AsyncCallbackWrapper<int>(callback);
    taskpool.execute((): void => {
      try {
        nativeGetUid(agent, call);
      } catch (err) {
        callback(err as BusinessError, -1);
      }
    });
  }

  export function getUid(agent: WantAgent): Promise<int> {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgent error. Agent must be a WantAgent.');
    }
    return new Promise<int>((resolve: (data: int) => void, reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<int>((err: BusinessError | null, data: int | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as int);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeGetUid(agent, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function getWant(agent: WantAgent, callback: AsyncCallback<Want>): void {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse wantAgent failed! Agent must be a WantAgent!');
    }
    let call = new AsyncCallbackWrapper<Want>(callback);
    taskpool.execute((): void => {
      try {
        nativeGetWant(agent, call);
      } catch (err) {
        callback(err as BusinessError, {} as Want);
      }
    });
  }

  export function getWant(agent: WantAgent): Promise<Want> {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse wantAgent failed! Agent must be a WantAgent!');
    }
    return new Promise<Want>((resolve: (data: Want) => void, reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<Want>((err: BusinessError | null, data: Want | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Want);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeGetWant(agent, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function cancel(agent: WantAgent, callback: AsyncCallback<void>): void {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgent error. Agent must be a WantAgent.');
    }
    let call = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      try {
        nativeCancel(agent, call);
      } catch (err) {
        callback(err as BusinessError, undefined);
      }
    });
  }

  export function cancel(agent: WantAgent): Promise<void> {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgent error. Agent must be a WantAgent.');
    }
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeCancel(agent, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function trigger(agent: WantAgent, triggerInfo: TriggerInfo, callback?: AsyncCallback<CompleteData>): void {
    nativeTriggerCheck(agent, triggerInfo);
    taskpool.execute((): void => {
      if (callback == undefined) {
        nativeTrigger(agent, triggerInfo);
      } else {
        let call = new AsyncCallbackWrapper<CompleteData>(callback);
        try {
          nativeTrigger(agent, triggerInfo, call);
        } catch (err) {
          callback(err as BusinessError, {info: new Object(), want: new Want(), finalCode: 0, finalData: 'undefined'} as CompleteData);
        }
      }
    });
  }

  export function equal(agent: WantAgent, otherAgent: WantAgent, callback: AsyncCallback<boolean>): void {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgentFirst failed. Agent must be a WantAgent.');
    }
    if (!(otherAgent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgentSceond failed. OtherAgent must be a WantAgent.');
    }
    let call = new AsyncCallbackWrapper<boolean>(callback);
    taskpool.execute((): void => {
      try {
        nativeEqual(agent, otherAgent, call);
      } catch (err) {
        callback(err as BusinessError, false);
      }
    });
  }

  export function equal(agent: WantAgent, otherAgent: WantAgent): Promise<boolean> {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgentFirst failed. Agent must be a WantAgent.');
    }
    if (!(otherAgent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse pWantAgentSceond failed. OtherAgent must be a WantAgent.');
    }
    return new Promise<boolean>((resolve: (data: boolean) => void,
      reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<boolean>((err: BusinessError | null, data: boolean | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as boolean);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeEqual(agent, otherAgent, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function getWantAgent(info: WantAgentInfo, callback: AsyncCallback<WantAgent>): void {
    let call = new AsyncCallbackWrapper<WantAgent>(callback);
    taskpool.execute((): void => {
      try {
        nativeGetWantAgent(info, call);
      } catch (err) {
        callback(err as BusinessError, {} as WantAgent);
      }
    });
  }

  export function getWantAgent(info: WantAgentInfo): Promise<WantAgent> {
    return new Promise<WantAgent>((resolve: (data: WantAgent) => void,
      reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<WantAgent>((err: BusinessError | null, data: WantAgent | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as WantAgent);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeGetWantAgent(info, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function getOperationType(agent: WantAgent, callback: AsyncCallback<int>): void {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse wantAgent failed! Agent must be a WantAgent.');
    }
    let call = new AsyncCallbackWrapper<int>(callback);
    taskpool.execute((): void => {
      try {
        nativeGetOperationType(agent, call);
      } catch (err) {
        callback(err as BusinessError, -1);
      }
    });
  }

  export function getOperationType(agent: WantAgent): Promise<int> {
    if (!(agent instanceof WantAgentCls)) {
      throw AbilityUtils.createBusinessError(ERROR_CODE_INVALID_PARAM, 'Parse wantAgent failed! Agent must be a WantAgent.');
    }
    return new Promise<int>((resolve: (data: int) => void, reject: (err: BusinessError) => void): void => {
      let call = new AsyncCallbackWrapper<int>((err: BusinessError | null, data: int | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as int);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        try {
          nativeGetOperationType(agent, call);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    });
  }

  export function isLocalWantAgent(agent: WantAgent): boolean {
    return nativeIsLocalWantAgent(agent);
  }

  export function createLocalWantAgent(info: LocalWantAgentInfo): WantAgent{
    return nativeCreateLocalWantAgent(info);
  }

  export enum WantAgentFlags {
    ONE_TIME_FLAG = 0,
    NO_BUILD_FLAG,
    CANCEL_PRESENT_FLAG,
    UPDATE_PRESENT_FLAG,
    CONSTANT_FLAG,
    REPLACE_ELEMENT,
    REPLACE_ACTION,
    REPLACE_URI,
    REPLACE_ENTITIES,
    REPLACE_BUNDLE
  }

  export enum OperationType {
    UNKNOWN_TYPE = 0,
    START_ABILITY,
    START_ABILITIES,
    START_SERVICE,
    SEND_COMMON_EVENT,
    START_SERVICE_EXTENSION = 6
  }

  export interface CompleteData {
    info: WantAgent;
    want: Want;
    finalCode: int;
    finalData: string;
    extraInfo?: Record<string, RecordData>;
  }

  class CompleteDataImpl implements CompleteData {
    public info: WantAgent;
    public want: Want;
    public finalCode: int;
    public finalData: string;
    public extraInfo?: Record<string, RecordData>;
  }

  export type TriggerInfo = _TriggerInfo;

  export type WantAgentInfo = _WantAgentInfo;

  export type LocalWantAgentInfo = _LocalWantAgentInfo;
}

export type WantAgent = object;

export default wantAgent;
