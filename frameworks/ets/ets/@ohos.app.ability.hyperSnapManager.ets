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

export default namespace hyperSnapManager {
loadLibraryWithPermissionCheck("hyper_snap_manager_ani.z", "@ohos.app.ability.hyperSnapManager")

/**
 * 错误类型枚举。
 * @enum ErrorType
 * @syscap SystemCapability.AbilityManager.AbilityRuntime
 * @since 12
 */
export enum ErrorType {
    /** 快照制作失败。 */
    CREATE_SNAPSHOT = 0,
    /** 从快照孵化进程失败。 */
    FORK_FROM_SNAPSHOT = 1
}

/**
 * 快照错误码枚举。
 * @enum HyperSnapErrorCode
 * @syscap SystemCapability.AbilityManager.AbilityRuntime
 * @since 12
 */
export enum HyperSnapErrorCode {
    /** 成功（无错误）。 */
    ERR_OK = 0,
    /** 系统内部错误。 */
    ERR_SYSTEM_INNER = 1,
    /** 快照已存在。 */
    ERR_SNAPSHOT_EXIST = 2,
    /** 准备制作快照时，已有进程正在运行。 */
    ERR_PROCESS_IS_RUNNING = 3,
    /** 快照制作过程中，用于制作快照的进程被杀。 */
    ERR_SNAPSHOT_PROCESS_IS_DIED = 4,
    /** 快照制作过程中，用户启动应用导致中断。 */
    ERR_SNAPSHOT_IS_INTERRUPTED = 5,
    /** 存在非法的 binder。 */
    ERR_EXISTS_ILLEGAL_BINDER = 6,
    /** 上一个进程未完全退出。 */
    ERR_LAST_PROCESS_NOT_FULLY_EXITED = 7
}

/**
 * 快照错误信息接口。
 * @interface HyperSnapErrorInfo
 * @syscap SystemCapability.AbilityManager.AbilityRuntime
 * @since 12
 */
export interface HyperSnapErrorInfo {
    /** 错误码。 */
    code: HyperSnapErrorCode;
    /** 错误消息。 */
    msg: string;
    /** 错误发生时间，使用开机后经过的毫秒数（字符串格式），例如 "12345678"。 */
    occurTimeStamp: string;
}

// Internal implementation class instantiated by the native side and returned via the
// Promise wrapper below. Fields mirror the public HyperSnapErrorInfo interface.
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

// Native synchronous implementation: returns the result object on success and throws
// (ThrowInvalidParamError / ThrowError) on parameter or system errors.
export native function nativeGetLastError(errType: ErrorType): HyperSnapErrorInfoImpl;

/**
 * 获取上一次快照相关的错误信息。
 *
 * 应用启动后调用本接口，查询最近一次快照制作（ErrorType.CREATE_SNAPSHOT）或从快照孵化进程
 * （ErrorType.FORK_FROM_SNAPSHOT）失败的错误信息。当无错误记录时，返回 code 为 ERR_OK 的默认信息。
 *
 * @param errType 错误类型，指定查询哪种快照操作的错误。
 * @returns Promise 对象，resolve 时返回错误信息的详细内容。
 * @throws { BusinessError } 401 参数错误；16000050 系统内部错误。
 * @syscap SystemCapability.AbilityManager.AbilityRuntime
 * @since 12
 */
export function getLastError(errType: ErrorType): Promise<HyperSnapErrorInfo> {
    return new Promise<HyperSnapErrorInfo>((resolve, reject) => {
        try {
            resolve(hyperSnapManager.nativeGetLastError(errType));
        } catch (err) {
            reject(err);
        }
    });
}
} // namespace hyperSnapManager
