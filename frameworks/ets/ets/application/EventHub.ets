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

export type ESValueArgs = ESValue | Object | null | undefined
export type FixedArgs = Object | null | undefined

export default class EventHub {
  private nativeContext: long = 0;
  private isApplicationContext: boolean = false;
  private eventMap: Map<string, Function[]> = new Map<string, Function[]>();
  private dynamicContextEventHub?: ESValue;
  private native getDynamicContextEventHub(): ESValue;

  constructor(ptr: long) {
    this.nativeContext = ptr;
  }
  
  constructor() {
  }

  on(event: string, callback: Function): void {
    if (!this.eventMap.has(event)) {
      this.eventMap.set(event, []);
    }
    const callbacks = this.eventMap.get(event)!;
    let foundIndex = -1;
    for (let i = 0; i < callbacks.length; i++) {
      if (callbacks[i] == callback) {
        foundIndex = i;
        break;
      }
    }
    if (foundIndex == -1) {
      callbacks.push(callback);
    }
    if (!this.dynamicContextEventHub) {
      this.dynamicContextEventHub = this.getDynamicContextEventHub();
    }
  }

  offByNativeContext(event: string, callback?: Function): void {
    if (!this.eventMap.has(event)) {
      return;
    }
    if (!callback) {
      this.eventMap.delete(event);
    } else {
      const callbacks = this.eventMap.get(event)!;
      let foundIndex = -1;
      for (let i = 0; i < callbacks.length; i++) {
        if (callbacks[i] == callback) {
          foundIndex = i;
          break;
        }
      }
      if (foundIndex > -1) {
        callbacks.splice(foundIndex, 1);
      }
      if (callbacks.length === 0) {
        this.eventMap.delete(event);
      }
    }
  }

  off(event: string, callback: Function): void {
    console.log('AbilityKit eventHub off event: ' + event);
    this.offByNativeContext(event, callback);
    if (!this.dynamicContextEventHub) {
      this.dynamicContextEventHub = this.getDynamicContextEventHub();
    }
    const eventHub = this.dynamicContextEventHub?.getProperty("eventHub");
    if (eventHub && !eventHub.isUndefined()) {
      eventHub.invokeMethod("offByNativeContext", ESValue.wrap(event));
    }
  }

  offByDynamicContext(event: string, callback: Function): void {
    this.offByNativeContext(event, callback);
  }

  off(event: string): void {
    this.offByNativeContext(event, undefined);
    if (!this.dynamicContextEventHub) {
      this.dynamicContextEventHub = this.getDynamicContextEventHub();
    }
    const eventHub = this.dynamicContextEventHub?.getProperty("eventHub");
    if (eventHub && !eventHub.isUndefined()) {
      eventHub.invokeMethod("offByNativeContext", ESValue.wrap(event));
    }
  }

  offByDynamicContext(event: string): void {
    this.offByNativeContext(event, undefined);
  }

  private emitByStaticContext(event: string, ...args: FixedArray<(Object|null|undefined)>): void {
    if (!this.eventMap.has(event)) {
      return;
    }
    const callbacks = [...this.eventMap.get(event)!];
    callbacks.forEach(callback => {
      try {
        callback.unsafeCall(...args);
      } catch (err) {
        console.error(`Error '${event}'`, err);
      }
    });
  }
  
  emit(event: string, ...args: (Object|null|undefined)[]): void {
    let fixedArgs: FixedArray<(Object|null|undefined)> = new FixedArray<FixedArgs>(args.length, undefined);
    for (let i = 0; i < args.length; i++) {
      fixedArgs[i] = args[i];
    }
    this.emitByStaticContext(event, ...fixedArgs);
    if (!this.dynamicContextEventHub) {
      this.dynamicContextEventHub = this.getDynamicContextEventHub();
    }
    const eventHub = this.dynamicContextEventHub?.getProperty("eventHub");
    if (eventHub && !eventHub.isUndefined()) {
      let params: FixedArray<ESValueArgs> = new FixedArray<ESValueArgs>(args.length + 1, undefined);
      params[0] = ESValue.wrap(event);
      for (let i = 0; i < args.length; i++) {
        params[i + 1] = ESValue.wrap(args[i]);
      }
      eventHub.invokeMethod("emitByNativeContext", ...params);
    }
  }

  emitByDynamicContext(event: string, ...args: FixedArray<(Object|null|undefined)>): void {
    this.emitByStaticContext(event, ...args);
  }
}
