/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

import Want from '@ohos.app.ability.Want';
import wantConstant from '@ohos.app.ability.wantConstant';
import { BusinessError, AsyncCallback, RecordData } from '@ohos.base';
import insightIntent from '@ohos.app.ability.insightIntent';
import AsyncCallbackWrapper from './utils/AbilityUtils';

export default namespace insightIntentDriver {

  loadLibraryWithPermissionCheck("insight_intent_driver_ani_kit.z", "@ohos.app.ability.insightIntentDriver")

  export interface ExecuteParam {
    bundleName: string;
    moduleName: string;
    abilityName: string;
    insightIntentName: string;
    insightIntentParam: Record<string, RecordData>;
    executeMode: insightIntent.ExecuteMode;
    displayId?: long;
    uris?: Array<string>;
    flags?: int;
    userId?: int;
  }

  interface InsightIntentInfoFilter {
    intentFlags: int;
    bundleName?: string;
    moduleName?: string;
    intentName?: string;
    userId?: int;
  }
  enum InsightIntentType {
    LINK = '@InsightIntentLink',
    PAGE = '@InsightIntentPage',
    ENTRY = '@InsightIntentEntry',
    FUNCTION = '@InsightIntentFunctionMethod',
    FORM = '@InsightIntentForm',
  }

  enum ExecuteModeForConfiguration {  
    FOREGROUND = 0,
    BACKGROUND = 1
  }

  interface UIAbilityIntentInfo {
    readonly abilityName: string;
    readonly executeMode: ExecuteModeForConfiguration[];
  }

  class UIAbilityIntentInfoInner implements UIAbilityIntentInfo {
    readonly abilityName: string;
    readonly executeMode: ExecuteModeForConfiguration[];
  }

  interface UIExtensionIntentInfo {  
    readonly abilityName: string;
  }

  class UIExtensionIntentInfoInner implements UIExtensionIntentInfo {
    readonly abilityName: string;
  }

  interface ServiceExtensionIntentInfo {  
    readonly abilityName: string;
  }

  class ServiceExtensionIntentInfoInner implements ServiceExtensionIntentInfo {
    readonly abilityName: string;
  }

  enum DevelopType {  
    CONFIGURATION = 'configuration',
    DECORATOR = 'decorator'
  }

  interface SubIntentInfoForConfiguration {
    readonly srcEntry: string;
    readonly uiAbility?: UIAbilityIntentInfo;
    readonly uiExtension?: UIExtensionIntentInfo;
    readonly serviceExtension?: ServiceExtensionIntentInfo;
    readonly form?: FormIntentInfo;
    readonly inputParams?: Array<Record<string, RecordData>>;
    readonly outputParams?: Array<Record<string, RecordData>>;
    readonly entities?: Record<string, RecordData>;
  }

  class SubIntentInfoForConfigurationInner implements SubIntentInfoForConfiguration {
    readonly srcEntry: string;
    readonly uiAbility?: UIAbilityIntentInfo;
    readonly uiExtension?: UIExtensionIntentInfo;
    readonly serviceExtension?: ServiceExtensionIntentInfo;
    readonly form?: FormIntentInfo;
    readonly inputParams?: Array<Record<string, RecordData>>;
    readonly outputParams?: Array<Record<string, RecordData>>;
    readonly entities?: Record<string, RecordData>;
  }

  interface InsightIntentInfo {
    readonly bundleName: string;
    readonly moduleName: string;
    readonly intentName: string;
    readonly domain: string;
    readonly intentVersion: string;
    readonly displayName: string;
    readonly displayDescription: string;
    readonly schema: string;
    readonly icon: string;
    readonly llmDescription: string;
    readonly keywords: string[];
    readonly intentType: InsightIntentType;
    readonly subIntentInfo: LinkIntentInfo | PageIntentInfo | FunctionIntentInfo | FormIntentInfo | EntryIntentInfo;
    readonly parameters: Record<string, RecordData>;
    readonly result: Record<string, RecordData>;
    readonly entities: Array<EntityInfo>;
    readonly developType?: DevelopType;
    readonly subIntentInfoForConfiguration?: SubIntentInfoForConfiguration;
  }

  class InsightIntentInfoInner implements InsightIntentInfo { 
    readonly bundleName: string;
    readonly moduleName: string;
    readonly intentName: string;
    readonly domain: string;
    readonly intentVersion: string;
    readonly displayName: string;
    readonly displayDescription: string;
    readonly schema: string;
    readonly icon: string;
    readonly llmDescription: string;
    readonly keywords: string[];
    readonly intentType: InsightIntentType;
    readonly subIntentInfo: LinkIntentInfo | PageIntentInfo | FunctionIntentInfo | FormIntentInfo | EntryIntentInfo;
    readonly parameters: Record<string, RecordData>;
    readonly result: Record<string, RecordData>;
    readonly entities: Array<EntityInfo>;
    readonly developType?: DevelopType;
    readonly subIntentInfoForConfiguration?: SubIntentInfoForConfiguration;
  }

  interface LinkIntentInfo {
    readonly uri: string;
  }

  class LinkIntentInfoInner implements LinkIntentInfo {
    readonly uri: string;
  }

  interface PageIntentInfo {
    readonly uiAbility: string;
    readonly pagePath: string;
    readonly navigationId: string;
    readonly navDestinationName: string;
  }

  class PageIntentInfoInner implements PageIntentInfo {
    readonly uiAbility: string;
    readonly pagePath: string;
    readonly navigationId: string;
    readonly navDestinationName: string;
  }

  interface FunctionIntentInfo {}

  class FunctionIntentInfoInner implements FunctionIntentInfo {
  }

  interface FormIntentInfo {
    readonly abilityName: string;
    readonly formName: string;
  }
  class FormIntentInfoInner implements FormIntentInfo {
    readonly abilityName: string;
    readonly formName: string;
  }

  interface EntryIntentInfo {
    readonly abilityName: string;
    readonly executeMode: insightIntent.ExecuteMode[];
  }

  class EntryIntentInfoInner implements EntryIntentInfo {
    readonly abilityName: string;
    readonly executeMode: insightIntent.ExecuteMode[];
  }

  enum GetInsightIntentFlag {
    GET_FULL_INSIGHT_INTENT = 0x00000001,
    GET_SUMMARY_INSIGHT_INTENT = 0x00000002,
    GET_ENTITY_INFO = 0x00000004,
  }

  interface EntityInfo {
    readonly className: string;
    readonly entityId: string;
    readonly entityCategory: string;
    readonly parameters: Record<string, RecordData>;
    readonly parentClassName: string;
  }

  class EntityInfoInner implements EntityInfo {
    readonly className: string;
    readonly entityId: string;
    readonly entityCategory: string;
    readonly parameters: Record<string, RecordData>;
    readonly parentClassName: string;
  }

  export native function nativeExecuteSync(param: ExecuteParam,
    callback: AsyncCallbackWrapper<insightIntent.ExecuteResult>,
    isCallback: boolean): void;
  export native function nativeGetAllInsightIntentInfo(intentFlags: int, callback: AsyncCallbackWrapper<Array<InsightIntentInfo>>): void;
  export native function nativeGetAllInfoCheck(intentFlags: int): void;
  export native function nativeGetInsightIntentInfoByBundleName(bundleName: string, intentFlags: int, callback: AsyncCallbackWrapper<Array<InsightIntentInfo>>): void;
  export native function nativeGetInfoByBundleNameCheck(bundleName: string, intentFlags: int): void;
  export native function nativeGetInsightIntentInfoByIntentName(bundleName: string, moduleName: string, intentName: string, intentFlags: int, callback: AsyncCallbackWrapper<InsightIntentInfo>): void;
  export native function nativeGetInfoByIntentNameCheck(bundleName: string, moduleName: string, intentName: string, intentFlags: int): void;
  export native function nativeGetInsightIntentInfoByFilterCheck(filter: InsightIntentInfoFilter): void;
  export native function nativeGetInsightIntentInfoByFilter(filter: InsightIntentInfoFilter,
    callback: AsyncCallbackWrapper<Array<InsightIntentInfo>>): void;

  export function execute(param: ExecuteParam, callback: AsyncCallback<insightIntent.ExecuteResult>): void {
    let pCallback = new AsyncCallbackWrapper<insightIntent.ExecuteResult>(callback);
    taskpool.execute((): void => {
      nativeExecuteSync(param, pCallback, true);
    });
  }

  export function execute(param: ExecuteParam): Promise<insightIntent.ExecuteResult> {
    let pPromise = new Promise<insightIntent.ExecuteResult>((
      resolve: (data: insightIntent.ExecuteResult) => void,
      reject: (err: BusinessError) => void) => {
      let pCallback = new AsyncCallbackWrapper<insightIntent.ExecuteResult>((
        err: BusinessError | null, data: insightIntent.ExecuteResult | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as insightIntent.ExecuteResult);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        nativeExecuteSync(param, pCallback, false);
      });
    });
    return pPromise;
  }

  function getAllInsightIntentInfo(intentFlags: int): Promise<Array<InsightIntentInfo>> {
    nativeGetAllInfoCheck(intentFlags);
    return new Promise<Array<InsightIntentInfo>>((resolve: (data: Array<InsightIntentInfo>) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<Array<InsightIntentInfo>>((err: BusinessError | null,
        data: Array<InsightIntentInfo> | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Array<InsightIntentInfo>);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        nativeGetAllInsightIntentInfo(intentFlags, asyncCall);
      });
    });
  }

  function getInsightIntentInfoByBundleName(bundleName: string, intentFlags: int): Promise<Array<InsightIntentInfo>> {
    nativeGetInfoByBundleNameCheck(bundleName, intentFlags);
    return new Promise<Array<InsightIntentInfo>>((resolve: (data: Array<InsightIntentInfo>) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<Array<InsightIntentInfo>>((err: BusinessError | null,
        data: Array<InsightIntentInfo> | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Array<InsightIntentInfo>);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        nativeGetInsightIntentInfoByBundleName(bundleName, intentFlags, asyncCall);
      });
    });
  }

  function getInsightIntentInfoByIntentName(bundleName: string, moduleName: string, intentName: string, intentFlags: int): Promise<InsightIntentInfo> {
    nativeGetInfoByIntentNameCheck(bundleName, moduleName, intentName, intentFlags);
    return new Promise<InsightIntentInfo>((resolve: (data: InsightIntentInfo) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<InsightIntentInfo>((err: BusinessError | null,
        data: InsightIntentInfo | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as InsightIntentInfo);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        nativeGetInsightIntentInfoByIntentName(bundleName, moduleName, intentName, intentFlags, asyncCall);
      });
    });
  }

  export function getInsightIntentInfoByFilter(filter: InsightIntentInfoFilter):
    Promise<Array<InsightIntentInfo>> {
    nativeGetInsightIntentInfoByFilterCheck(filter);
    return new Promise<Array<InsightIntentInfo>>((resolve: (data: Array<InsightIntentInfo>) => void,
      reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<Array<InsightIntentInfo>>((err: BusinessError | null,
        data: Array<InsightIntentInfo> | undefined) => {
        if (err == null || err.code == 0) {
          resolve(data as Array<InsightIntentInfo>);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        nativeGetInsightIntentInfoByFilter(filter, asyncCall);
      });
    });
  }
}