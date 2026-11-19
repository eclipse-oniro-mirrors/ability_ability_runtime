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

import { AutoFillType } from 'application.AutoFillType';
import CustomData from 'application.CustomData';
import AutoFillPopupConfig from 'application.AutoFillPopupConfig';
import ViewData from 'application.ViewData';
import { AutoFillTriggerType } from 'application.AutoFillTriggerType';

export interface FillRequest {
  type: AutoFillType;
  viewData: ViewData;
  customData: CustomData;
  isPopup: boolean;
  triggerType?: AutoFillTriggerType;
}
export interface SaveRequest {
  viewData: ViewData;
}
export interface UpdateRequest {
  viewData: ViewData;
}
export interface FillResponse {
  viewData: ViewData;
}
export interface FillRequestCallback {
  onSuccess(response: FillResponse): void;
  onFailure(): void;
  onCancel(fillContent?: string): void;
  setAutoFillPopupConfig(autoFillPopupConfig: AutoFillPopupConfig): void;
}
export interface SaveRequestCallback {
  onSuccess(): void;
  onFailure(): void;
}
export interface FillFailureResult {
  errCode: int;
}

class FillRequestInner implements FillRequest {
  public type: AutoFillType = AutoFillType.UNSPECIFIED;
  public viewData!: ViewData;
  public customData: CustomData = {data: {}};
  public isPopup: boolean;
  public triggerType?: AutoFillTriggerType;
}
class SaveRequestInner implements SaveRequest {
  public viewData!: ViewData;
}
class UpdateRequestInner implements UpdateRequest {
  public viewData!: ViewData;
}
class FillResponseInner implements FillResponse {
  public viewData!: ViewData;
}
class FillFailureResultInner implements FillFailureResult {
  public errCode: int;
}

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

class FillRequestCallbackInner implements FillRequestCallback {
  fillRequestCallbackInnerPtr: long = 0;
  private cleaner: Cleaner | null = null;
  constructor(ptr: long) {
    if(this.fillRequestCallbackInnerPtr == 0){
          this.fillRequestCallbackInnerPtr = ptr;
    }
    this.registerCleaner(this.fillRequestCallbackInnerPtr)
  }
  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr)
      destroyRegister.register(this, this.cleaner!, unregisterToken);
  }
  unregisterCleaner(): void {
      destroyRegister.unregister(unregisterToken);
  }

  public native onSuccess(response: FillResponse): void;
  public native onFailure(): void;
  public native onCancel(fillContent?: string): void;
  public native setAutoFillPopupConfig(autoFillPopupConfig: AutoFillPopupConfig): void;
}

class SaveRequestCallbackInner implements SaveRequestCallback {
  saveRequestCallbackInnerPtr: long = 0;
  private cleaner: Cleaner | null = null;
  constructor(ptr: long) {
    if(this.saveRequestCallbackInnerPtr == 0){
          this.saveRequestCallbackInnerPtr = ptr;
    }
    this.registerCleaner(this.saveRequestCallbackInnerPtr)
  }
  registerCleaner(ptr: long): void {
      this.cleaner = new Cleaner(ptr)
      destroyRegister.register(this, this.cleaner!, unregisterToken);
  }
  unregisterCleaner(): void {
      destroyRegister.unregister(unregisterToken);
  }

  public native onSuccess(): void;
  public native onFailure(): void;
}