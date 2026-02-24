/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import AtomicServiceOptions from '@ohos.app.ability.AtomicServiceOptions';
import { BusinessError } from '@ohos.base';
import { Caller } from '@ohos.app.ability.UIAbility';
import { ConnectOptions } from 'ability.connectOptions';
import ExtensionContext from 'application.ExtensionContext';
import OpenLinkOptions from '@ohos.app.ability.OpenLinkOptions';
import StartOptions from '@ohos.app.ability.StartOptions';
import Want from '@ohos.app.ability.Want';

class Cleaner {
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

export default class ServiceExtensionContext extends ExtensionContext {
  static {
    loadLibraryWithPermissionCheck("service_extension_context_ani_kit.z", "application.ServiceExtensionContext");
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

  private static contextType: string = 'ServiceExtensionContext';

  static transferStatic(input: Any): Object {
    let type: string = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (err) {
      console.log('transferStatic err: ' + err.toString());
    }
    console.log('transferStatic type: ' + type);
    return ServiceExtensionContext.nativeTransferStatic(ESValue.wrap(input), type);
  }

  static transferDynamic(input: Object): Any {
    return ServiceExtensionContext.nativeTransferDynamic(input).unwrap();
  }

  native nativeTerminateSelf(callback: AsyncCallbackWrapper<void>): void;

  native nativeStartAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartAbility(want: Want, options: StartOptions, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeStopServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeConnectServiceExtensionAbility(want: Want, options: ConnectOptions): long;

  native nativeConnectServiceExtensionAbilityWithAccount(want: Want, accountId: int, options: ConnectOptions): long;

  native nativeDisconnectServiceExtensionAbility(connection: long, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartUIServiceExtensionAbility(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeWantCheck(want: Want): void;
  native nativeStartUIAbilities(wantList: Array<Want>, callback: AsyncCallbackWrapper<void>): void;
  native nativeOpenAtomicService(appId: string, callback: AsyncCallbackWrapper<void>, options?: AtomicServiceOptions): void;

  native nativePreStartMission(bundleName: string, moduleName: string, abilityName: string, startTime: string, callback: AsyncCallbackWrapper<void>): void;

  native nativeRequestModalUIExtension(want: Want, callback: AsyncCallbackWrapper<void>): void;

  native nativeStopServiceExtensionAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartServiceExtensionAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartRecentAbility(want: Want, callback: AsyncCallbackWrapper<void>, options?: StartOptions): void;

  native nativeStartAbilityWithAccountSync(want: Want, accountId: int, callback: AsyncCallbackWrapper<void>): void;

  native nativeStartAbilityWithAccountSync(want: Want, accountId: int, options: StartOptions, callback: AsyncCallbackWrapper<void>): void;

  native nativeOpenLink(link: string, callback: AsyncCallbackWrapper<void>, options?: OpenLinkOptions): void;
  native nativeOpenLinkCheck(link: string): void;

  native nativeStartAbilityByCallWithAccount(want: Want, accountId: int): Caller;

  native nativeStartAbilityByCallSync(want: Want): Caller;

  native nativeStartAbilityAsCaller(want: Want, callback: AsyncCallbackWrapper<void>, options?: StartOptions): void;

  terminateSelf(callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeTerminateSelf(myCall);
    });
  }

  terminateSelf(): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeTerminateSelf(myCall);
        });
      });
  }

  startAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, myCall);
    });
  }

  startAbility(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbility(want, options, myCall);
    });
  }

  startAbility(want: Want, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        if (options == null) {
          this.nativeStartAbility(want, myCall);
        } else {
          this.nativeStartAbility(want, options as StartOptions, myCall);
        }
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  startServiceExtensionAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartServiceExtensionAbility(want, myCall);
    });
  }

  startServiceExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartServiceExtensionAbility(want, myCall);
        });
      });
  }

  stopServiceExtensionAbility(want: Want, callback: AsyncCallback<void>): void {
    let myCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStopServiceExtensionAbility(want, myCall);
    });
  }

  stopServiceExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStopServiceExtensionAbility(want, myCall);
      });
    });
  }

  connectServiceExtensionAbility(want: Want, options: ConnectOptions): long {
    return this.nativeConnectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbility(connection: long, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeDisconnectServiceExtensionAbility(connection, asyncCall);
    });
  }

  disconnectServiceExtensionAbility(connection: long): Promise<void> {
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

  startUIServiceExtensionAbility(want: Want): Promise<void> {
    this.nativeWantCheck(want);
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartUIServiceExtensionAbility(want, myCall);
        });
    });
  };

  openAtomicService(appId: string, options?: AtomicServiceOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let asyncCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeOpenAtomicService(appId, asyncCall, options);
      }).catch((err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

 startUIAbilities(wantList: Array<Want>): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
      let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeStartUIAbilities(wantList, myCall);
      });
    });
  }
  preStartMission(bundleName: string, moduleName: string, abilityName: string, startTime: string): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativePreStartMission(bundleName, moduleName, abilityName, startTime, myCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  requestModalUIExtension(pickerWant: Want, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeRequestModalUIExtension(pickerWant, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  requestModalUIExtension(pickerWant: Want): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeRequestModalUIExtension(pickerWant, myCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  startRecentAbility(want: Want, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartRecentAbility(want, myCall, options);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  startRecentAbility(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartRecentAbility(want, asyncCall, options);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startRecentAbility(want: Want, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartRecentAbility(want, asyncCall, undefined);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityByCallWithAccount(want: Want, accountId: int): Promise<Caller> {
    return taskpool.execute((): Caller => {
      return this.nativeStartAbilityByCallWithAccount(want, accountId);
    }) as Promise<Caller>;
  }

  startAbilityByCall(want: Want): Promise<Caller> {
    return taskpool.execute((): Caller => {
      return this.nativeStartAbilityByCallSync(want);
    }) as Promise<Caller>;
  }

  connectServiceExtensionAbilityWithAccount(want: Want, accountId: int, options: ConnectOptions): long {
    return this.nativeConnectServiceExtensionAbilityWithAccount(want, accountId, options);
  }

  stopServiceExtensionAbilityWithAccount(want: Want, accountId: int): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStopServiceExtensionAbilityWithAccount(want, accountId, myCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  stopServiceExtensionAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStopServiceExtensionAbilityWithAccount(want, accountId, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startServiceExtensionAbilityWithAccount(want: Want, accountId: int): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartServiceExtensionAbilityWithAccount(want, accountId, myCall);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  startServiceExtensionAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartServiceExtensionAbilityWithAccount(want, accountId, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityWithAccount(want: Want, accountId: int, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          if (options == null) {
            this.nativeStartAbilityWithAccountSync(want, accountId, myCall);
          } else {
            this.nativeStartAbilityWithAccountSync(want, accountId, options as StartOptions, myCall);
          }
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  startAbilityWithAccount(want: Want, accountId: int, options: StartOptions, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityWithAccountSync(want, accountId, options, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityWithAccount(want: Want, accountId: int, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityWithAccountSync(want, accountId, asyncCall);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityAsCaller(want: Want, options?: StartOptions): Promise<void> {
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeStartAbilityAsCaller(want, myCall, options);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }

  startAbilityAsCaller(want: Want, options: StartOptions, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityAsCaller(want, asyncCall, options);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  startAbilityAsCaller(want: Want, callback: AsyncCallback<void>): void {
    let asyncCall = new AsyncCallbackWrapper<void>(callback);
    taskpool.execute((): void => {
      this.nativeStartAbilityAsCaller(want, asyncCall, undefined);
    }).catch((err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  openLink(link: string, options?: OpenLinkOptions): Promise<void> {
    this.nativeOpenLinkCheck(link);
    return new Promise<void>((resolve: (data: undefined) => void, reject: (err: BusinessError) => void): void => {
        let myCall = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
          if (err == null || err.code == 0) {
            resolve(undefined);
          } else {
            reject(err);
          }
        });
        taskpool.execute((): void => {
          this.nativeOpenLink(link, myCall, options);
        }).catch((err: Error): void => {
          reject(err as BusinessError);
        });
      });
  }
}
