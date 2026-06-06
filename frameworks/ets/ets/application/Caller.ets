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
import { BusinessError } from '@ohos.base';
import { Caller, OnReleaseCallback, OnRemoteStateChangeCallback } from '@ohos.app.ability.UIAbility';

const domainID = 0x0000;
const TAG = 'UIAbility';

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;

const ERROR_CODE_INVALID_PARAM: int = 401;
const ERROR_CODE_CALLER_RELEASED: int = 16200001;
const ERROR_CODE_CALLEE_INVALID: int = 16200002;
const ERROR_CODE_INNER_ERROR: int = 16000050;
const ERROR_CODE_TRANSFER_CLASS_NOT_FOUND: int = 10200067;

const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
const ERROR_MSG_CALLER_RELEASED = 'Caller released. The caller has been released.';
const ERROR_MSG_CALLEE_INVALID = 'The callee does not exist.';
const ERROR_MSG_INNER_ERROR = 'Inner Error.';
const ERROR_MSG_TRANSFER_CLASS_NOT_FOUND = 'Unable to find the class for transferring.';

let errMap = new Map<int, string>();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_CALLER_RELEASED, ERROR_MSG_CALLER_RELEASED);
errMap.set(ERROR_CODE_CALLEE_INVALID, ERROR_MSG_CALLEE_INVALID);
errMap.set(ERROR_CODE_INNER_ERROR, ERROR_MSG_INNER_ERROR);
errMap.set(ERROR_CODE_TRANSFER_CLASS_NOT_FOUND, ERROR_MSG_TRANSFER_CLASS_NOT_FOUND);

function buildError(code: int): BusinessError {
  if (errMap.has(code)) {
    return new BusinessError(code, new Error(errMap.get(code) as string));
  }
  return new BusinessError(ERROR_CODE_INNER_ERROR, new Error(ERROR_MSG_INNER_ERROR));
}

function buildInvalidParamError(msg: string): BusinessError {
  return new BusinessError(ERROR_CODE_INVALID_PARAM, new Error(ERROR_MSG_INVALID_PARAM + msg));
}

function buildInnerError(msg: string): BusinessError {
  return new BusinessError(ERROR_CODE_INNER_ERROR, new Error(ERROR_MSG_INNER_ERROR + ' ' + msg));
}

export class CallerImpl implements Caller {
  static {
    loadLibrary("caller_complex_ani");
  }
  private nativeCaller: long = 0;
  private callee: rpc.IRemoteObject | null = null;
  private releaseState = false;
  private esObj: ESValue | null = null; // used for transfer
  private releaseCb: OnReleaseCallback = (msg: String) => {};
  private stateChangeCb: OnRemoteStateChangeCallback = (msg: String) => {};

  private native nativeReleaseSync(): void;
  private static native nativeTransferStatic(input: ESValue): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;

  static transferStatic(input: Any): Object {
    let esObj = ESValue.wrap(input);
    let releaseState = esObj.getProperty('releaseState').toBoolean();
    hilog.info(domainID, TAG, `Caller::transferStatic releaseState: ${releaseState}`);
    if (releaseState) {
      throw buildError(ERROR_CODE_TRANSFER_CLASS_NOT_FOUND);
    }
    let ret = CallerImpl.nativeTransferStatic(esObj.getProperty('__call_obj__')) as CallerImpl;
    ret.esObj = esObj;
    return ret;
  }

  static transferDynamic(input: Object): Any {
    let caller = input as CallerImpl;
    if (caller.releaseState) {
      throw buildError(ERROR_CODE_TRANSFER_CLASS_NOT_FOUND);
    }
    if (caller.esObj != null) {
      return (caller.esObj as ESValue).unwrap();
    }
    let esObj = CallerImpl.nativeTransferDynamic(input);
    caller.esObj = esObj;
    return esObj.unwrap();
  }

  constructor(nativeCaller: long) {
    this.nativeCaller = nativeCaller;
    hilog.info(domainID, TAG, 'Caller::constructor');
  }

  private callCheck(method: string, data: rpc.Parcelable): BusinessError | null {
    if (method.length == 0) {
      hilog.info(domainID, TAG, 'empty method');
      return buildInvalidParamError('Parameter error: method or data is empty, Please check it.');
    }

    if (this.releaseState) {
      hilog.info(domainID, TAG, 'Caller callCheck this.callee release');
      return buildError(ERROR_CODE_CALLER_RELEASED);
    }

    if (this.callee == null) {
      hilog.info(domainID, TAG, 'Caller callCheck this.callee is null');
      return buildError(ERROR_CODE_CALLEE_INVALID);
    }
    return null;
  }

  private buildMsgData(method: string, data: rpc.Parcelable): rpc.MessageSequence {
    let msgData = rpc.MessageSequence.create();
    msgData.writeString(method);
    msgData.writeParcelable(data);
    return msgData;
  }

  async call(method: string, data: rpc.Parcelable): Promise<void> {
    hilog.info(domainID, TAG, 'Caller call');
    const checkError = this.callCheck(method, data);
    if (checkError != null) {
      throw checkError;
    }

    hilog.info(domainID, TAG, 'Caller call msgData rpc.MessageSequence create');
    let msgData = this.buildMsgData(method, data);
    let msgReply = rpc.MessageSequence.create();
    try {
      let remoteObj = this.callee as rpc.IRemoteObject;
      let retData = await remoteObj.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply,
        new rpc.MessageOption());
      hilog.info(domainID, TAG, 'Caller call msgData rpc.sendMessageRequest called');
      if (retData.errCode != REQUEST_SUCCESS) {
        hilog.info(domainID, TAG, 'Caller call return errCode ' + retData.errCode);
        throw buildInnerError('Rpc send message error. Try again later.');
      }
      let retVal = retData.reply.readInt();
      let str = retData.reply.readString();
      if (retVal != REQUEST_SUCCESS || str != 'object') {
        hilog.info(domainID, TAG, 'call retVal is [' + retVal + '], str [' + str + ']');
        throw buildError(retVal);
      }
    } catch (error) {
      hilog.info(domainID, TAG, 'Caller call msgData rpc.sendMessageRequest error ' + error);
      throw error;
    } finally {
      msgData.reclaim();
      msgReply.reclaim();
    }
  }

  async callWithResult(method: string, data: rpc.Parcelable): Promise<rpc.MessageSequence> {
    hilog.info(domainID, TAG, 'Caller callWithResult');
    const checkError = this.callCheck(method, data);
    if (checkError != null) {
      throw checkError;
    }

    hilog.info(domainID, TAG, 'Caller callWithResult msgData rpc.MessageSequence create');
    let msgData = this.buildMsgData(method, data);
    let msgReply = rpc.MessageSequence.create();
    try {
      let remoteObj = this.callee as rpc.IRemoteObject;
      let retData = await remoteObj.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply,
        new rpc.MessageOption());
      hilog.info(domainID, TAG, 'Caller callWithResult msgData rpc.sendMessageRequest called');
      if (retData.errCode != REQUEST_SUCCESS) {
        hilog.info(domainID, TAG, 'Caller callWithResult return errCode ' + retData.errCode);
        throw buildInnerError('Rpc send message error. Try again later.');
      }
      let retVal = retData.reply.readInt();
      let str = retData.reply.readString();
      if (retVal != REQUEST_SUCCESS || str != 'object') {
        hilog.info(domainID, TAG, 'callWithResult retVal is [' + retVal + '], str [' + str + ']');
        throw buildError(retVal);
      }
      return retData.reply;
    } catch (error) {
      hilog.info(domainID, TAG, 'callWithResult msgData rpc.sendMessageRequest error ' + error);
      msgReply.reclaim();
      throw error;
    } finally {
      msgData.reclaim();
    }
  }

  release(): void {
    hilog.info(domainID, TAG, 'Caller release called.');
    if (this.releaseState === true) {
      hilog.info(domainID, TAG, 'Caller release remoteObj releaseState is true');
      throw buildError(ERROR_CODE_CALLER_RELEASED);
    }

    if (this.callee == null) {
      hilog.info(domainID, TAG, 'Caller release call remoteObj is released');
      throw buildError(ERROR_CODE_CALLEE_INVALID);
    }

    this.releaseState = true;
    this.callee = null;
    this.nativeReleaseSync();
    this.nativeCaller = 0;
  }

  onRelease(callback: OnReleaseCallback): void {
    hilog.info(domainID, TAG, 'onRelease called');
    if (this.releaseState) {
      throw buildError(ERROR_CODE_CALLER_RELEASED);
    }
    this.releaseCb = callback;
  }
  private onReleaseCb(msg: string): void {
    this.releaseCb(msg);
  }

  onRemoteStateChange(callback: OnRemoteStateChangeCallback): void {
    hilog.info(domainID, TAG, 'onRemoteStateChange called');
    if (this.releaseState) {
      throw buildError(ERROR_CODE_CALLER_RELEASED);
    }
    this.stateChangeCb = callback;
  }
  private onRemoteChangeCb(msg: string): void {
    this.stateChangeCb(msg);
  }

  on(type: string, callback: OnReleaseCallback): void {	
    hilog.info(domainID, TAG, 'on called');
    if (type != 'release') {
      throw buildInvalidParamError('Parameter error: Failed to get type, must be string type release.');
    }
    this.onRelease(callback);
  }

  offRelease(callback: OnReleaseCallback): void {
    hilog.info(domainID, TAG, 'off called');
    // do nothing
  }
  offRelease(): void {
    hilog.info(domainID, TAG, 'off called');
    // do nothing
  }
}