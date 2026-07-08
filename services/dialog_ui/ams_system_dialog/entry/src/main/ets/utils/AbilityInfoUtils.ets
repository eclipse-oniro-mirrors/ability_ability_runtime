/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import common from '@ohos.app.ability.common';

const TAG = 'SelectorExtensionAbility.AbilityInfoUtils';

export namespace AbilityInfoUtils {
    interface AbilityInfoParam {
        bundleName: string;
        moduleName: string;
        abilityName?: string;
        abilityIconId: number;
        abilityLabelId: number;
    }

    export function getAbilityLabelAndIcon(context: common.UIExtensionContext, abilityInfoParam: AbilityInfoParam,
                                           callback: (data: Object[] | null) => void) {
        let moduleContext: Context | undefined =
        context?.createModuleContext(abilityInfoParam.bundleName, abilityInfoParam.moduleName);
        if (!moduleContext) {
            throw { code: 1000, data: 'Get module context error.' } as BusinessError<string> as Error;
        }

        const resourceManager = moduleContext.resourceManager;
        Promise.all<Object>([resourceManager.getDrawableDescriptor(abilityInfoParam.abilityIconId),
        resourceManager.getStringValue(abilityInfoParam.abilityLabelId)]).then((infoArray: Object[]) => {
            callback(infoArray);
        })
    }

    export function getAbilityLabelAndIconSync(context: common.UIExtensionContext, abilityInfoParam: AbilityInfoParam,
                                               callback: (data: Object[] | null) => void) {

        let moduleContext: Context | undefined = context.createModuleContext(abilityInfoParam.bundleName,
            abilityInfoParam.moduleName);
        if (!moduleContext) {
            throw { code: 1000, data: 'Get module context error.' } as BusinessError<string> as Error;
        }
        const resourceManager = moduleContext.resourceManager;
        try {
            let icon = resourceManager.getDrawableDescriptor(abilityInfoParam.abilityIconId);
            let label = resourceManager.getStringSync(abilityInfoParam.abilityLabelId);
            callback([icon, label]);
        } catch (err) {
            console.error(TAG, `Failed to getAbilityLabelAndIconSync, error: ${JSON.stringify(err)}`);
            callback(null);
        }
    }
}