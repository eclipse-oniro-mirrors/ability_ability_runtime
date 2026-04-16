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

export default namespace errorManager {
    loadLibrary("errorManager_ani");
    export type ErrorHandler = (errObject: Error) => void;
    export native function setDefaultErrorHandler(defaultHandler?: ErrorHandler): ErrorHandler;

    export type FreezeObserver = () => void;
    export native function onFreeze(observer: FreezeObserver): void;
    export native function offFreeze(observer?: FreezeObserver): void;

    export type UnhandledRejectionObserver = (reason: Error | Any, promise: Promise<Any>) => void;
    native function notifyUnhandledRejectionHandler(reason: Error | Any, promise: Promise<Any>): void;
    native function onUnhandledRejectionInner(observer: UnhandledRejectionObserver): void;
    export function onUnhandledRejection(observer: UnhandledRejectionObserver): void {
        StdProcess.on('unhandledPromiseRejection', (reason: Error, obj: Object): void => {
            notifyUnhandledRejectionHandler(reason, obj as Promise<Any>);
        });
        onUnhandledRejectionInner(observer);
    };
    export native function offUnhandledRejection(observer?: UnhandledRejectionObserver): void;
    export native function setDefaultFreezeObserver(defaultObserver?: FreezeObserver) : FreezeObserver;
}
