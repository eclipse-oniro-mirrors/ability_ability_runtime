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

import hilog from '@ohos.hilog';
import rpc from '@ohos.rpc';
import transfer from '@ohos.transfer';
import { BusinessError } from '@ohos.base';
import { Callee, CalleeCallback } from '@ohos.app.ability.UIAbility';

const domainID = 0x0000;
const TAG = 'UIAbility';

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;
const REQUEST_FAILED = 1;

const ERROR_CODE_INVALID_PARAM: int = 401;
const ERROR_CODE_FUNC_REGISTERED: int = 16200004;
const ERROR_CODE_FUNC_NOT_EXIST: int = 16200005;
const ERROR_CODE_INNER_ERROR: int = 16000050;
const ERROR_CODE_TRANSFER_CLASS_NOT_FOUND: int = 10200067;

const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
const ERROR_MSG_FUNC_REGISTERED = 'Method registered. The method has registered.';
const ERROR_MSG_FUNC_NOT_EXIST = 'The method has not been registered.';
const ERROR_MSG_INNER_ERROR = 'Inner Error.';
const ERROR_MSG_TRANSFER_CLASS_NOT_FOUND = 'Unable to find the class for transferring.';

let errMap = new Map<int, string>();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_FUNC_REGISTERED, ERROR_MSG_FUNC_REGISTERED);
errMap.set(ERROR_CODE_FUNC_NOT_EXIST, ERROR_MSG_FUNC_NOT_EXIST);
errMap.set(ERROR_CODE_INNER_ERROR, ERROR_MSG_INNER_ERROR);
errMap.set(ERROR_CODE_TRANSFER_CLASS_NOT_FOUND, ERROR_MSG_TRANSFER_CLASS_NOT_FOUND);

function buildError(code: int): BusinessError {
  if (errMap.has(code)) {
    return new BusinessError(code, new Error(errMap.get(code) as string));
  }
  return new BusinessError(ERROR_CODE_INNER_ERROR, new Error(ERROR_MSG_INNER_ERROR));
}

class EstParcelable {
  private etsObj: rpc.Parcelable;
  constructor(etsObj: rpc.Parcelable) {
    this.etsObj = etsObj;
  }
  marshalling(dataOut: Any): boolean {
    let data = transfer.transferStatic(dataOut, 'rpc.MessageSequence') as rpc.MessageSequence;
    return this.etsObj.marshalling(data);
  }
  unmarshalling(dataIn: Any): boolean {
    let data = transfer.transferStatic(dataIn, 'rpc.MessageSequence') as rpc.MessageSequence;
    return this.etsObj.unmarshalling(data);
  }
}

class CalleeFromJs implements Callee {
  esObj: ESValue;
  constructor(jsObj: Any) {
    this.esObj = ESValue.wrap(jsObj);
  }
  on(method: string, callback: CalleeCallback): void {
    if (CoroutineExtras.isMainWorker()) {
      this.esObj.invokeMethod('on', method, (seq: Any): EstParcelable => {
        let data = transfer.transferStatic(seq, 'rpc.MessageSequence') as rpc.MessageSequence;
        return new EstParcelable(callback(data));
      });
      return;
    }
    EAWorker.postToMain<void>((): void => {
      this.esObj.invokeMethod('on', method, (seq: Any): EstParcelable => {
        let data = transfer.transferStatic(seq, 'rpc.MessageSequence') as rpc.MessageSequence;
        return new EstParcelable(callback(data));
      });
    });
  }

  off(method: string): void {
    if (CoroutineExtras.isMainWorker()) {
      this.esObj.invokeMethod('off', method);
      return;
    }
    EAWorker.postToMain<void>((): void => {
      this.esObj.invokeMethod('off', method);
    });
  }
}

class JsParcelable implements rpc.Parcelable {
  private esObj: ESValue;
  constructor(jsObj: Any) {
    this.esObj = ESValue.wrap(jsObj);
  }
  marshalling(dataOut: rpc.MessageSequence): boolean {
    if (CoroutineExtras.isMainWorker()) {
      let ret = this.esObj.invokeMethod('marshalling',
        ESValue.wrap(transfer.transferDynamic(dataOut, 'rpc.MessageSequence')));
      return ret.toBoolean();
    }
    let job = EAWorker.postToMain<boolean>((): boolean => {
      let ret = this.esObj.invokeMethod('marshalling',
        ESValue.wrap(transfer.transferDynamic(dataOut, 'rpc.MessageSequence')));
      return ret.toBoolean();
    });
    return job.Await();
  }
  unmarshalling(dataIn: rpc.MessageSequence): boolean {
    if (CoroutineExtras.isMainWorker()) {
      let ret = this.esObj.invokeMethod('unmarshalling',
        ESValue.wrap(transfer.transferDynamic(dataIn, 'rpc.MessageSequence')));
      return ret.toBoolean();
    }
    let job = EAWorker.postToMain<boolean>((): boolean => {
      let ret = this.esObj.invokeMethod('unmarshalling',
        ESValue.wrap(transfer.transferDynamic(dataIn, 'rpc.MessageSequence')));
      return ret.toBoolean();
    });
    return job.Await();
  }
}

class CalleeFromEts {
  callee: Callee;
  constructor(callee: Callee) {
    this.callee = callee;
  }
  on(method: string, callback: (indata: Any) => Any): void {
    this.callee.on(method, (indata: rpc.MessageSequence): rpc.Parcelable => {
      if (CoroutineExtras.isMainWorker()) {
        let retData = callback(transfer.transferDynamic(indata, 'rpc.MessageSequence'));
        if (retData instanceof rpc.Parcelable) {
          return retData;
        }
        return new JsParcelable(retData);
      }
      let job = EAWorker.postToMain<rpc.Parcelable>((): rpc.Parcelable => {
        let retData = callback(transfer.transferDynamic(indata, 'rpc.MessageSequence'));
        if (retData instanceof rpc.Parcelable) {
          return retData;
        }
        return new JsParcelable(retData);
      });
      return job.Await();
    });
  }

  off(method: string): void {
    this.callee.off(method);
  }
}

export class CalleeImpl extends rpc.RemoteObject implements Callee {
  private callList = new Map<string, CalleeCallback>();
  private startUpNewRule = false;
  constructor() {
    super('rpc.application.callee');
    hilog.info(domainID, TAG, 'Callee::constructor');
  }

  static transferStatic(input: Any): Object {
    hilog.info(domainID, TAG, 'Callee::transferStatic');
    if (input instanceof CalleeFromEts) {
      return input.callee;
    }
    return new CalleeFromJs(input);
  }

  static transferDynamic(input: Object): Any {
    hilog.info(domainID, TAG, 'Callee::transferDynamic');
    if (input instanceof CalleeFromJs) {
      return input.esObj.unwrap();
    }
    if (input instanceof CalleeImpl) {
      return new CalleeFromEts(input);
    }
    throw buildError(ERROR_CODE_TRANSFER_CLASS_NOT_FOUND);
  }

  private setNewRuleFlag(flag: boolean): void {
    hilog.info(domainID, TAG, 'Callee::setNewRuleFlag: ' + flag);
    this.startUpNewRule = flag;
  }
  private StartUpRuleCheck(): boolean {
    return true;
  }

  onRemoteMessageRequest(code: int, data: rpc.MessageSequence, reply: rpc.MessageSequence,
    option: rpc.MessageOption): boolean {
    hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest code [' + code + ']');
    if (!this.StartUpRuleCheck()) {
      return false;
    }

    if (code != EVENT_CALL_NOTIFY) {
      hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest error, code is ' + code);
      return false;
    }

    if (this.callList.size == 0) {
      hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest error, this.callList is empty');
      return false;
    }
    let method = data.readString();
    hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest method [' + method + ']');
    let func = this.callList.get(method);
    if (typeof func != 'function') {
      hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest error, get func is ' + typeof func);
      return false;
    }

    let callback = func as CalleeCallback;
    let result = callback(data);
    if (typeof result == 'object' && result != null) {
      reply.writeInt(REQUEST_SUCCESS);
      reply.writeString(typeof result);
      reply.writeParcelable(result);
      hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest code proc Packed data');
    } else {
      reply.writeInt(REQUEST_FAILED);
      reply.writeString(typeof result);
      hilog.info(domainID, TAG, 'Callee onRemoteMessageRequest error, retVal is ' +
        REQUEST_FAILED + ', type is ' + typeof result);
    }
    return true;
  }

  on(method: string, callback: CalleeCallback): void {
    if (method.length == 0) {
      hilog.info(domainID, TAG, 'Callee::on method is empty');
      throw buildError(ERROR_CODE_INVALID_PARAM);
    }

    if (this.callList.has(method)) {
      hilog.info(domainID, TAG, 'Callee on error, [' + method + '] has registered');
      throw buildError(ERROR_CODE_FUNC_REGISTERED);
    }

    this.callList.set(method, callback);
    hilog.info(domainID, TAG, 'Callee on method [' + method + ']');
  }

  off(method: string): void {
    if (method.length == 0) {
      hilog.info(domainID, TAG, 'Callee::on method is empty');
      throw buildError(ERROR_CODE_INVALID_PARAM);
    }

    if (!this.callList.has(method)) {
      hilog.info(domainID, TAG, 'Callee off error, this.callList not found ' + method);
      throw buildError(ERROR_CODE_FUNC_NOT_EXIST);
    }

    this.callList.delete(method);
    hilog.info(domainID, TAG, 'Callee off method [' + method + ']');
  }
}