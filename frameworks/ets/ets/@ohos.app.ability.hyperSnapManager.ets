/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base';
import AsyncCallbackWrapper from './utils/AbilityUtils';

export default namespace hyperSnapManager {
loadLibraryWithPermissionCheck("hyper_snap_manager_ani.z", "@ohos.app.ability.hyperSnapManager")

export enum ErrorType {
    CREATE_SNAPSHOT = 0,
    FORK_FROM_SNAPSHOT = 1
}

export enum HyperSnapErrorCode {
    ERR_OK = 0,
    ERR_SYSTEM_INNER = 1,
    ERR_SNAPSHOT_EXIST = 2,
    ERR_PROCESS_IS_RUNNING = 3,
    ERR_SNAPSHOT_PROCESS_IS_DIED = 4,
    ERR_SNAPSHOT_IS_INTERRUPTED = 5,
    ERR_EXISTS_ILLEGAL_BINDER = 6,
    ERR_LAST_PROCESS_NOT_FULLY_EXITED = 7
}

export interface HyperSnapErrorInfo {
    code: HyperSnapErrorCode;
    msg: string;
    occurTimeStamp: string;
}

class HyperSnapErrorInfoImpl implements HyperSnapErrorInfo {
    code: HyperSnapErrorCode = HyperSnapErrorCode.ERR_OK;
    msg: string = '';
    occurTimeStamp: string = '';
}

export native function nativeSetHyperSnapEnabled(enableFlag: boolean): void;
export function setHyperSnapEnabled(enableFlag: boolean): void {
    hyperSnapManager.nativeSetHyperSnapEnabled(enableFlag);
}

export native function nativeRequestRebuildHyperSnap(): void;
export function requestRebuildHyperSnap(): void {
    hyperSnapManager.nativeRequestRebuildHyperSnap();
}

export native function nativeGetLastError(errType: ErrorType, callback: AsyncCallbackWrapper<HyperSnapErrorInfo>): void;
export function getLastError(errType: ErrorType): Promise<HyperSnapErrorInfo> {
    return new Promise<HyperSnapErrorInfo>((resolve, reject) => {
        let call = new AsyncCallbackWrapper<HyperSnapErrorInfo>((err: BusinessError | null,
            data: HyperSnapErrorInfo | undefined) => {
            if (err == null || err.code == 0) {
                resolve(data as HyperSnapErrorInfo);
            } else {
                reject(err);
            }
        });
        taskpool.execute((): void => {
            try {
                hyperSnapManager.nativeGetLastError(errType, call);
            } catch (err) {
                reject(err as BusinessError);
            }
        });
    });
}
} // namespace hyperSnapManager
