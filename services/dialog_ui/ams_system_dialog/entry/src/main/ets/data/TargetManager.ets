/*
 *
 *  * Copyright (c) 2024 Huawei Device Co., Ltd.
 *  * Licensed under the Apache License, Version 2.0 (the "License");
 *  * you may not use this file except in compliance with the License.
 *  * You may obtain a copy of the License at
 *  *
 *  *     http://www.apache.org/licenses/LICENSE-2.0
 *  *
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS,
 *  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  * See the License for the specific language governing permissions and
 *  * limitations under the License.
 *
 */
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import HashMap from '@ohos.util.HashMap';
import Want from '@ohos.app.ability.Want';
import { TargetAction, TargetInfo, TargetType, UIExtensionInfo } from './Model';
import bundleManager from '@ohos.bundle.bundleManager';

const TAG = 'SelectorExtensionAbility.TargetManager';

export namespace TargetManager {
    interface TargetActionBuilderParam {
        bundleName: string;
        abilityName: string;
        uri?: string;
        targetType: number;
    }

    const availableTargetsWithCallWant: HashMap<Want, Array<TargetInfo>> = new HashMap();

    export function getAvailableTargets(callerWant: Want): Array<TargetInfo> {
        if (availableTargetsWithCallWant.hasKey(callerWant)) {
            console.info(TAG, `Get targets from cache.`)
            return availableTargetsWithCallWant.get(callerWant);
        }
        let extensionInfos: bundleManager.ExtensionAbilityInfo[] = [];
        if (!(callerWant?.parameters?.['ability.want.params.uiExtensionTargetType'])) {
            return [];
        }
        try {
            extensionInfos = 
                bundleManager.queryExtensionAbilityInfoSync(
                    callerWant.parameters['ability.want.params.uiExtensionTargetType'] as string,
                    bundleManager.ExtensionAbilityFlag.GET_EXTENSION_ABILITY_INFO_DEFAULT);
        } catch (e) {
            console.info(TAG, `there are no target uiextension: ${e}`)
        }
        return extensionInfos.map(extensionInfo => {
            return new TargetInfo(extensionInfo, buildUIExtensionTargetAction(callerWant, {
                bundleName: extensionInfo.bundleName,
                abilityName: extensionInfo.name,
                targetType: TargetType.UI_EXTENSION,
                uri: ''
            },))
        })
    }

    function buildUIExtensionTargetAction(callerWant: Want, params: TargetActionBuilderParam): TargetAction {
        if (callerWant.parameters) {
            callerWant.parameters['ability.want.params.uiExtensionType'] = callerWant.parameters['ability.want.params.uiExtensionTargetType'];
            callerWant.parameters['ability.want.params.uiExtensionTargetType'] = '';
        }
        return new TargetAction(TargetType.UI_EXTENSION, (storage?: LocalStorage) => {
            return new UIExtensionInfo(params.bundleName, params.abilityName, callerWant.parameters)
        })
    }

    export function removePartialTargetListIfExceedsMax(targetInfoArray: Array<TargetInfo>, maxSize: number): 
        Array<TargetInfo> {
        if (targetInfoArray.length <= maxSize) {
            return targetInfoArray;
        }
        const removedResult: TargetInfo[] = targetInfoArray.slice(0, maxSize - 1);
        removedResult.push(new TargetInfo(null, new TargetAction(TargetType.SHOW_ALL_TARGET, null)))
        return removedResult;
    }
}