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

export default namespace appMemoryOptimizer {
    loadLibraryWithPermissionCheck("app_memory_optimizer_ani_kit.z", "@ohos.app.ability.appMemoryOptimizer");

    export native function nativeEvictFilePagesSync(fileNames: Array<string>,
        callback: AsyncCallbackWrapper<void>): void;

    export native function nativeEvictModuleFilePagesSync(moduleNames: Array<string>,
        callback: AsyncCallbackWrapper<void>): void;

    export native function nativeEvictFilePagesCheck(fileNames: Array<string>): void;

    export native function nativeEvictModuleFilePagesCheck(moduleNames: Array<string>): void;

    export function evictFilePages(fileNames: Array<string>): Promise<void> {
        appMemoryOptimizer.nativeEvictFilePagesCheck(fileNames);
        let p = new Promise<void>((resolve: (data: undefined) => void,
            reject: (err: BusinessError) => void): void => {
            let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
                if (err == null || err.code === 0) {
                    resolve(undefined);
                } else {
                    reject(err);
                }
            });
            taskpool.execute((): void => {
                appMemoryOptimizer.nativeEvictFilePagesSync(fileNames, myCall);
            });
        });
        return p;
    }

    export function evictModuleFilePages(moduleNames: Array<string>): Promise<void> {
        appMemoryOptimizer.nativeEvictModuleFilePagesCheck(moduleNames);
        let p = new Promise<void>((resolve: (data: undefined) => void,
            reject: (err: BusinessError) => void): void => {
            let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
                if (err == null || err.code === 0) {
                    resolve(undefined);
                } else {
                    reject(err);
                }
            });
            taskpool.execute((): void => {
                appMemoryOptimizer.nativeEvictModuleFilePagesSync(moduleNames, myCall);
            });
        });
        return p;
    }
}
