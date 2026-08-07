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
import { RecordData } from '@ohos.base';

namespace insightIntent {

  export enum ExecuteMode {
    UI_ABILITY_FOREGROUND = 0,
    UI_ABILITY_BACKGROUND = 1,
    UI_EXTENSION_ABILITY = 2,
    SERVICE_EXTENSION_ABILITY = 3,
  }

  export interface InteractionUI {
    interactionUIType: string;
  }

  export interface InteractionModalUIExtension extends InteractionUI {
    interactionUIType: 'MODAL_UIEXTENSION';
    bundleName: string;
    moduleName: string;
    abilityName: string;
    uiExtensionType: string;
    uri: string;
    parameters?: Record<string, RecordData>;
  }

  export interface InteractionInfo {
    interactionUI?: InteractionUI;
  }

  export interface ExecuteResult {
    code: int;
    result?: Record<string, RecordData>;
    uris?: Array<string>;
    flags?: int;
    interactionInfo?: InteractionInfo;
  }

  export interface IntentResult<T> {
    code: int;
    result?: T;
    interactionInfo?: InteractionInfo;
  }

  export class ExecuteResultInner implements ExecuteResult{
    code: int;
    result?: Record<string, RecordData>;
    uris?: Array<string>;
    flags?: int;
    interactionInfo?: InteractionInfo;
  }

  export class InteractionUIInner implements InteractionUI {
    interactionUIType: string = '';
  }

  export class InteractionModalUIExtensionInner implements InteractionModalUIExtension {
    interactionUIType: 'MODAL_UIEXTENSION' = 'MODAL_UIEXTENSION';
    bundleName: string = '';
    abilityName: string = '';
    moduleName: string = '';
    uiExtensionType: string = '';
    uri: string = '';
    parameters?: Record<string, RecordData>;
  }

  export class InteractionInfoInner implements InteractionInfo {
    interactionUI?: InteractionUI = new InteractionUIInner();
  }

  export enum ReturnMode {
    CALLBACK = 0,
    FUNCTION = 1
  }

  export enum QueryType {
    ALL = 'all',
    BY_PROPERTY = 'byProperty'
  }

  export interface QueryEntityParam {
    queryType: QueryType;
    parameters?: Record<string, RecordData>;
  }

  export class QueryEntityParamInner implements QueryEntityParam {
    queryType: QueryType;
    parameters?: Record<string, RecordData>;
  }

  interface IntentEntity {
    entityId: string;
  }

  abstract class AppIntentEntity<T> implements IntentEntity {
    displayName: string;
    abstract onQueryEntity(params: QueryEntityParam): Promise<Array<T>>;

    public nativeExecutor: long = 0;
    public native nativeOnQueryEntityResult(data: Array<Record<string, RecordData>>): void;
    public native nativeOnQueryEntityError(err: Error): void;

    public callPromise(p: Promise<Array<T>>): void {
      p.then((result: Array<T>): void => {
        this.nativeOnQueryEntityResult(this.entityArrayToRecord(result));
      }).catch((err: Error): void => {
        this.nativeOnQueryEntityError(err);
      });
    }

    private entityArrayToRecord(entityArr: Array<T>): Array<Record<string, RecordData>> {
      let recordArr: Array<Record<string, RecordData>> = [];
      for (let entity of entityArr) {
        recordArr.push(this.entityToRecord(entity));
      }
      return recordArr;
    }

    private entityToRecord(entity: T): Record<string, RecordData> {
      let jsonStr: string = JSON.stringify(entity);
      let jsonValue = JSON.parseJsonElement(jsonStr);
      return this.jsonElementToRecord(jsonValue);
    }

    private jsonElementToRecord(jsonValue: jsonx.JsonElement): Record<string, RecordData> {
      let record: Record<string, RecordData> = new Record<string, RecordData>();
      if (jsonValue.jsonType != jsonx.JsonType.JsonObject) {
        return record;
      }
      let iterVal: IterableIterator<[string, jsonx.JsonElement]> = jsonValue.$_iterator();
      for (let entry of iterVal) {
        let key = entry[0];
        let value = entry[1];
        if (key == 'nativeExecutor') {
          continue;
        }
        switch (value.jsonType) {
          case jsonx.JsonType.JsonString:
            record[key] = value.asString();
            break;
          case jsonx.JsonType.JsonNumber:
            record[key] = value.asDouble();
            break;
          case jsonx.JsonType.JsonTrue:
            record[key] = new Boolean(true);
            break;
          case jsonx.JsonType.JsonFalse:
            record[key] = new Boolean(false);
            break;
          case jsonx.JsonType.JsonNull:
            record[key] = null;
            break;
          case jsonx.JsonType.JsonArray:
            let values: Array<jsonx.JsonElement> = value.asArray();
            let result: Array<Record<string, RecordData>> = new Array<Record<string, RecordData>>();
            for (let i: int = 0; i < values.length; i++) {
              result.push(this.jsonElementToRecord(values[i]));
            }
            record[key] = result;
            break;
          case jsonx.JsonType.JsonObject:
            record[key] = this.jsonElementToRecord(value);
            break;
          default:
            throw new TypeError('unknown JSONElement type');
        }
      }
      return record;
    }
  }
}

export default insightIntent;