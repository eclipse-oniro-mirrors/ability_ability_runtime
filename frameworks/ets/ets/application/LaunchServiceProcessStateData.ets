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

import launchServiceManager from '../@ohos.app.ability.launchServiceManager';
import LaunchServiceDescriptorInner, { LaunchServiceDescriptor } from './LaunchServiceDescriptor';

export interface LaunchServiceProcessStateData {
    descriptor: LaunchServiceDescriptor;
    pid: int;
    state: launchServiceManager.LaunchServiceProcessState;
}

export default class LaunchServiceProcessStateDataInner implements LaunchServiceProcessStateData {
    descriptor: LaunchServiceDescriptor = new LaunchServiceDescriptorInner();
    pid: int = 0;
    state: launchServiceManager.LaunchServiceProcessState = launchServiceManager.LaunchServiceProcessState.CREATED;
}
