/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

/**
 * @file
 * @kit AbilityKit
 */

import Want from '@ohos.app.ability.Want';

namespace dialogRequest {
  loadLibrary("dialogrequest_ani_kit.z");

  class Cleaner {
    public ptr: long = 0;

    constructor(ptr: long) {
      this.ptr = ptr;
    }

    native cleanToReqInfo(): void;
    native cleanToReqCallback(): void;
  }

  export function callbackToReqInfo(cleaner: Cleaner): void {
    cleaner.cleanToReqInfo();
  }
  export function callbackToReqCallback(cleaner: Cleaner): void {
    cleaner.cleanToReqCallback();
  }

  let destroyRegisterReqInfo: FinalizationRegistry<Cleaner> = new FinalizationRegistry<Cleaner>(callbackToReqInfo);
  let unregisterTokenReqInfo: object = new object();

  let destroyRegisterReqCallback: FinalizationRegistry<Cleaner> = new FinalizationRegistry<Cleaner>(callbackToReqCallback);
  let unregisterTokenReqCallback: object = new object();
  
  export interface WindowRect {
    left: int;
    top: int;
    width: int;
    height: int;
  }

  export class WindowRectInner implements WindowRect {
    left: int;
    top: int;
    width: int;
    height: int;
  }

  export interface RequestInfo {
    windowRect?: WindowRect
  }

  export class RequestInfoInner implements RequestInfo {
    nativeRequestInfo: long = 0;
    private cleaner: Cleaner | null = null;

    constructor(ptr: long) {
      if (this.nativeRequestInfo == 0) {
        this.nativeRequestInfo = ptr;
      }
      this.registerCleaner(this.nativeRequestInfo);
    }

    registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr);
      destroyRegisterReqInfo.register(this, this.cleaner!, unregisterTokenReqInfo);
    }

    unregisterCleaner(): void {
      destroyRegisterReqInfo.unregister(unregisterTokenReqInfo);
    }
    windowRect?: WindowRect;
  }
  
  export enum ResultCode {
    RESULT_OK = 0,
    RESULT_CANCEL = 1
  }

  export interface RequestResult {
    result: ResultCode;
    want?: Want;
  }

  export class RequestResultInner implements RequestResult {
    result: ResultCode;
    want?:Want;
  }

  export interface RequestCallback {
    setRequestResult(result: RequestResult): void;
  }

  export class RequestCallbackInner implements RequestCallback {
    nativeRequestCallback:long = 0;
    private cleaner: Cleaner | null = null;

    constructor(ptr: long) {
      if (this.nativeRequestCallback == 0) {
        this.nativeRequestCallback = ptr;
      }
      this.registerCleaner(this.nativeRequestCallback);
    }

    registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr);
      destroyRegisterReqCallback.register(this, this.cleaner!, unregisterTokenReqCallback);
    }

    unregisterCleaner(): void {
      destroyRegisterReqCallback.unregister(unregisterTokenReqCallback);
    }
    public native setRequestResult(result: RequestResult): void;
  }

  export native function getRequestInfo(want: Want): RequestInfo;
  export native function getRequestCallback(want: Want): RequestCallback;
}

export default dialogRequest;