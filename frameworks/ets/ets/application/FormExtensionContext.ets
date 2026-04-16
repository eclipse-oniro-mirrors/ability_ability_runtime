/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { AsyncCallback } from '@ohos.base';
import AsyncCallbackWrapper from '../utils/AbilityUtils';
import { BusinessError } from '@ohos.base';
import { ConnectOptions } from 'ability.connectOptions';
import ExtensionContext from 'application.ExtensionContext';
import Want from '@ohos.app.ability.Want';

export class Cleaner {
  public ptr: long = 0;

  constructor(ptr: long) {
    this.ptr = ptr;
  }

  native clean(): void;
}

export function callback(cleaner: Cleaner): void {
  cleaner.clean();
}

let destroyRegister = new FinalizationRegistry<Cleaner>(callback);
let unregisterToken = new object();

export default class FormExtensionContext extends ExtensionContext {
  static {
    loadLibrary("form_extension_context_ani_kit.z");
  }

  constructor(context: long) {
    if (this.nativeEtsContext == 0) {
      this.nativeEtsContext = context;
    }
    this.registerCleaner(this.nativeEtsContext);
  }

  registerCleaner(ptr: long): void {
    this.cleaner = new Cleaner(ptr);
    destroyRegister.register(this, this.cleaner!, unregisterToken);
  }

  unregisterCleaner(): void {
    destroyRegister.unregister(unregisterToken);
  }

  nativeEtsContext: long = 0;
  private cleaner: Cleaner | null = null;

  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'FormExtensionContext';

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    console.log('transferStatic type: ' + type);
    return FormExtensionContext.nativeTransferStatic(ESValue.wrap(input), type);
  }

  static transferDynamic(input: Object): Any {
    return FormExtensionContext.nativeTransferDynamic(input).unwrap();
  }

  native nativeStartAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeConnectServiceExtensionAbility(want: Want, options: ConnectOptions): long;

  native nativeDisconnectServiceExtensionAbility(connection: long, callback: AsyncCallbackWrapper<void>): void;

  native nativeCheckWant(want: Want): void;

  native nativeCheckConnectionAbility(): void;

  startAbility(want: Want, callback: AsyncCallback<void>): void {
    this.nativeCheckWant(want);
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, myCall);
    });
  }

  startAbility(want: Want): Promise<void> {
    this.nativeCheckWant(want);
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartAbility(want, myCall);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  connectServiceExtensionAbility(want: Want, options: ConnectOptions): long {
    return this.nativeConnectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbility(connection: long, callback: AsyncCallback<void>): void {
    this.nativeCheckConnectionAbility();
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeDisconnectServiceExtensionAbility(connection, asyncCall);
    });
  }

  disconnectServiceExtensionAbility(connection: long): Promise<void> {
    this.nativeCheckConnectionAbility();
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeDisconnectServiceExtensionAbility(connection, asyncCall);
      })
    });
  }
}