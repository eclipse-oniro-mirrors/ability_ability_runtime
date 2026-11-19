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

import { UIContext } from '@ohos.arkui.UIContext';
import _ViewData from 'application.ViewData';
import _PageNodeInfo from 'application.PageNodeInfo';
import { FillRequest as _FillRequest, SaveRequest as _SaveRequest} from 'application.AutoFillRequest';
import { UpdateRequest as _UpdateRequest, FillResponse as _FillResponse} from 'application.AutoFillRequest';
import { FillRequestCallback as _FillRequestCallback } from 'application.AutoFillRequest';
import { SaveRequestCallback as _SaveRequestCallback } from 'application.AutoFillRequest';
import { FillFailureResult as _FillFailureResult } from 'application.AutoFillRequest';
import _CustomData from 'application.CustomData';
import _AutoFillRect from 'application.AutoFillRect';
import _AutoFillPopupConfig from 'application.AutoFillPopupConfig';
import { PopupSize as _PopupSize } from 'application.AutoFillPopupConfig';
import { AutoFillType as _AutoFillType } from 'application.AutoFillType';
import { PopupPlacement as _PopupPlacement } from 'application.AutoFillPopupConfig';

namespace autoFillManager {
  export type OnSuccessFn = () => void;
  export type OnFailureFn = () => void;
  export type OnFillSuccessFn = (viewData: ViewData) => void;
  export type OnFillFailureFn = (result: FillFailureResult) => void;

  loadLibraryWithPermissionCheck("ability_auto_fill_manager_ani_kit.z", "@ohos.app.ability.autoFillManager");

  export interface AutoSaveCallback {
    onSuccess: OnSuccessFn;
    onFailure: OnFailureFn;
  }

  class AutoSaveCallbackInner implements AutoSaveCallback {
    onSuccess: OnSuccessFn = () => {};
    onFailure: OnFailureFn = () => {};
  }

  export interface AutoFillCallback {
    onSuccess: OnFillSuccessFn;
    onFailure: OnFillFailureFn;
  }

  class AutoFillCallbackInner implements AutoFillCallback {
    onSuccess: OnFillSuccessFn = (viewData: ViewData) => {};
    onFailure: OnFillFailureFn = (result: FillFailureResult) => {};
  }

  export native function requestAutoSaveWithScope(callback?: AutoSaveCallback): void;
  export native function requestAutoSaveWithRequest(request: SaveRequest, callback?: AutoSaveCallback): void;
  export native function requestAutoFillWithScope(request: FillRequest, callback?: AutoFillCallback): void;

  export function requestAutoSave(context: UIContext, callback?: AutoSaveCallback): void {
    context.runScopedTask(() => {
      requestAutoSaveWithScope(callback);
    });
  }

  export function requestAutoSave(context: UIContext, request: SaveRequest, callback?: AutoSaveCallback): void {
    context.runScopedTask(() => {
      requestAutoSaveWithRequest(request, callback);
    });
  }

  export function requestAutoFill(context: UIContext, request: FillRequest, callback?: AutoFillCallback): void {
    context.runScopedTask(() => {
      requestAutoFillWithScope(request, callback);
    });
  }

  export type ViewData = _ViewData;
  export type PageNodeInfo = _PageNodeInfo;
  export type AutoFillType = _AutoFillType;
  export type FillRequest = _FillRequest;
  export type SaveRequest = _SaveRequest;
  export type UpdateRequest = _UpdateRequest;
  export type FillResponse = _FillResponse;
  export type FillRequestCallback = _FillRequestCallback;
  export type SaveRequestCallback = _SaveRequestCallback;
  export type FillFailureResult = _FillFailureResult;
  export type CustomData = _CustomData;
  export type AutoFillRect = _AutoFillRect;
  export type AutoFillPopupConfig = _AutoFillPopupConfig;
  export type PopupSize = _PopupSize;
  export type PopupPlacement = _PopupPlacement;
}

export default autoFillManager;
