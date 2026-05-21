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
import bundleManager from '@ohos.bundle.bundleManager';

export type UIAbilityActionExecution = (storage?: LocalStorage) => void

export type UIExtensionActionFunction = (storage?: LocalStorage) => UIExtensionInfo

export type CustomActionFunction = () => void

export class UIExtensionInfo {
    public readonly bundleName: string;
    public readonly abilityName: string;
    public readonly parameters?: Record<string, Object>;

    constructor(bundleName: string, abilityName: string, parameters?: Record<string, Object>) {
        this.bundleName = bundleName;
        this.abilityName = abilityName;
        this.parameters = parameters;
    }
}

export enum TargetType {
    UI_EXTENSION = 1,
    UI_ABILITY = 2,
    DEEPLINK = 3,
    SHOW_ALL_TARGET = 4
}

export enum PickerType {
    PHOTO_EDIT = 1,
    APP_SELECTOR = 2
}

export enum IntentType {
    PHOTO_EDIT = 1,
    GENERAL_APP_SELECTOR = 2
}

export class TargetAction {
    public readonly type: TargetType;
    public readonly actionExecution: UIAbilityActionExecution | UIExtensionActionFunction | CustomActionFunction | null;

    constructor(type: TargetType, actionExecution: UIAbilityActionExecution 
            | UIExtensionActionFunction | CustomActionFunction | null) {
        this.type = type;
        this.actionExecution = actionExecution;
    }
}

export class TargetInfo {
    public readonly filterAbilityInfo: bundleManager.ExtensionAbilityInfo | null;
    public readonly targetAction: TargetAction | null;
    public readonly visible: boolean;

    constructor(filterAbilityInfo: bundleManager.ExtensionAbilityInfo | null, targetAction: TargetAction | null,
                visible: boolean=true) {
        this.filterAbilityInfo = filterAbilityInfo;
        this.targetAction = targetAction;
        this.visible = visible;
    }
}

export class AbilityInfoParam {
    public bundleName: string;
    public moduleName: string;
    public abilityName: string | null;
    public abilityIconId: number;
    public abilityLabelId: number;

    constructor(bundleName: string, moduleName: string,
                abilityName: string, abilityIconId: number, abilityLabelId: number) {
        this.bundleName = bundleName;
        this.moduleName = moduleName;
        this.abilityName = abilityName;
        this.abilityIconId = abilityIconId;
        this.abilityLabelId = abilityLabelId;
    }
}

export interface IconAndLabelContent {
    icon: PixelMap;
    label: string;
}