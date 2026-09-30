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
import AsyncCallbackWrapper from './utils/AbilityUtils';
import { BusinessError } from '@ohos.base';

export default namespace taskManager {
loadLibraryWithPermissionCheck("taskmanager_ani.z", "@ohos.app.agent.taskManager")

export enum TaskPriority {
    LOW = 100,
    MEDIUM = 200,
    HIGH = 300
}

export native function nativeStartTask(taskId: string, priority: TaskPriority,
    callback: AsyncCallbackWrapper<void>): void;
export function startTask(taskId: string, priority?: TaskPriority): Promise<void> {
    let finalPriority: TaskPriority = (priority == undefined) ? TaskPriority.MEDIUM : priority;
    return new Promise<void>((resolve, reject) => {
        let call = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
            if (err == null || err.code == 0) {
                resolve(undefined);
            } else {
                reject(err);
            }
        });
        taskpool.execute((): void => {
            try {
                taskManager.nativeStartTask(taskId, finalPriority, call);
            } catch (err) {
                reject(err as BusinessError);
            }
        });
    });
}

export native function nativeFinishTask(taskId: string, callback: AsyncCallbackWrapper<void>): void;
export function finishTask(taskId: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        let call = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
            if (err == null || err.code == 0) {
                resolve(undefined);
            } else {
                reject(err);
            }
        });
        taskpool.execute((): void => {
            try {
                taskManager.nativeFinishTask(taskId, call);
            } catch (err) {
                reject(err as BusinessError);
            }
        });
    });
}
} // namespace taskManager
