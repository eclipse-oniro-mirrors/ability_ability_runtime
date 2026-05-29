/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
let rpc = requireNapi('rpc');
let hilog = requireNapi('hilog');

let domainID = 0xD001320;
let TAG = 'JSENV';

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_CODE_CALLER_RELEASED = 16200001;
const ERROR_CODE_CLAAEE_INVALID = 16200002;
const ERROR_CODE_INNER_ERROR = 16000050;

const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
const ERROR_MSG_CALLER_RELEASED = 'The caller has been released.';
const ERROR_MSG_CLAAEE_INVALID = 'The callee does not exist.';
const ERROR_MSG_INNER_ERROR = 'Inner Error.';

let errMap = new Map();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_CALLER_RELEASED, ERROR_MSG_CALLER_RELEASED);
errMap.set(ERROR_CODE_CLAAEE_INVALID, ERROR_MSG_CLAAEE_INVALID);
errMap.set(ERROR_CODE_INNER_ERROR, ERROR_MSG_INNER_ERROR);

class BusinessError extends Error {
  constructor(code) {
    let msg = '';
    if (errMap.has(code)) {
      msg = errMap.get(code);
    } else {
      msg = ERROR_MSG_INNER_ERROR;
    }
    super(msg);
    this.code = code;
  }
}

class ThrowInvalidParamError extends Error {
  constructor(msg) {
    let message = ERROR_MSG_INVALID_PARAM + msg;
    super(message);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class InnerCodeError extends Error {
  constructor(msg) {
    let message = ERROR_MSG_INNER_ERROR + ' ' + msg;
    super(message);
    this.code = ERROR_CODE_INNER_ERROR;
  }
}

class Caller {
  constructor(obj) {
    hilog.sLogI(domainID, TAG, 'Caller::constructor obj is ' + typeof obj);
    this.__call_obj__ = obj;
    this.releaseState = false;
  }

  call(method, data) {
    return new Promise(async (resolve, reject) => {
      const checkError = this.callCheck(method, data);
      if (checkError != null) {
        reject(checkError);
        return;
      }

      hilog.sLogI(domainID, TAG, 'Caller call msgData rpc.MessageSequence create');
      let msgData = this.buildMsgData(method, data);
      let msgReply = rpc.MessageSequence.create();

      let retData = undefined;
      try {
        retData = await this.__call_obj__.callee.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply,
          rpc.MessageOption());
        hilog.sLogI(domainID, TAG, 'Caller call msgData rpc.sendMessageRequest called');
        if (retData.errCode !== 0) {
          msgData.reclaim();
          msgReply.reclaim();
          hilog.sLogI(domainID, TAG, 'Caller call return errCode ' + retData.errCode);
          reject(new BusinessError(retData.errCode));
          return;
        }
      } catch (e) {
        hilog.sLogI(domainID, TAG, 'Caller call msgData rpc.sendMessageRequest error ' + e);
      }

      try {
        let retval = retData.reply.readInt();
        let str = retData.reply.readString();
        if (retval === REQUEST_SUCCESS && str === 'object') {
          msgData.reclaim();
          msgReply.reclaim();
        } else {
          hilog.sLogD(domainID, TAG,
            'Caller call retval is [' + retval + '], str [' + str + ']');
          msgData.reclaim();
          msgReply.reclaim();
          reject(new BusinessError(retval));
          return;
        }
      } catch (e) {
        hilog.sLogI(domainID, TAG, 'Caller call msgData sendMessageRequest retval error');
        msgData.reclaim();
        msgReply.reclaim();
        reject(new InnerCodeError('Read reply message error. Try again later.'));
        return;
      }

      hilog.sLogI(domainID, TAG, 'Caller call msgData sendMessageRequest end');
      resolve(undefined);
      return;
    });
  }

  callWithResult(method, data) {
    return new Promise(async (resolve, reject) => {
      hilog.sLogI(domainID, TAG, 'Caller callWithResult method [' + method + ']');
      const checkError = this.callCheck(method, data);
      if (checkError != null) {
        reject(checkError);
        return;
      }

      hilog.sLogI(domainID, TAG, 'Caller callWithResult msgData rpc.MessageSequence create');
      let msgData = this.buildMsgData(method, data);
      let msgReply = rpc.MessageSequence.create();

      let reply = undefined;
      let retData = undefined;
      try {
        retData = await this.__call_obj__.callee.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply,
          rpc.MessageOption());
        hilog.sLogI(domainID, TAG, 'Caller callWithResult msgData rpc.sendMessageRequest called');
        if (retData.errCode !== 0) {
          msgData.reclaim();
          msgReply.reclaim();
          hilog.sLogI(domainID, TAG, 'Caller callWithResult return errCode ' + retData.errCode);
          reject(new BusinessError(retData.errCode));
          return;
        }
      } catch (e) {
        hilog.sLogI(domainID, TAG, 'Caller call msgData rpc.MessageSequence error ' + e);
      }

      try {
        let retval = retData.reply.readInt();
        let str = retData.reply.readString();
        if (retval === REQUEST_SUCCESS && str === 'object') {
          msgData.reclaim();
          reply = retData.reply;
        } else {
          hilog.sLogI(domainID, TAG,
            'Caller callWithResult retval is [' + retval + '], str [' + str + ']');
          msgData.reclaim();
          msgReply.reclaim();
          reject(new BusinessError(retval));
          return;
        }
      } catch (e) {
        hilog.sLogI(domainID, TAG, 'Caller callWithResult msgData sendMessageRequest retval error');
        msgData.reclaim();
        msgReply.reclaim();
        reject(new InnerCodeError('Read reply message error. Try again later.'));
        return;
      }

      hilog.sLogI(domainID, TAG, 'Caller callWithResult msgData sendMessageRequest end');
      resolve(reply);
      return;
    });
  }

  release() {
    hilog.sLogI(domainID, TAG, 'Caller release js called.');
    if (this.releaseState === true) {
      hilog.sLogI(domainID, TAG, 'Caller release remoteObj releaseState is true');
      throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
    }

    if (this.__call_obj__.callee == null) {
      hilog.sLogI(domainID, TAG, 'Caller release call remoteObj is released');
      throw new BusinessError(ERROR_CODE_CLAAEE_INVALID);
    }

    this.releaseState = true;
    this.__call_obj__.release();
  }

  onRelease(callback) {
    hilog.sLogI(domainID, TAG, 'Caller onRelease jscallback called.');
    if (typeof callback !== 'function') {
      hilog.sLogI(domainID, TAG, 'Caller onRelease ' + typeof callback);
      throw new ThrowInvalidParamError('Parameter error: Failed to get callback, must be a function.');
    }

    if (this.releaseState === true) {
      hilog.sLogI(domainID, TAG, 'Caller onRelease remoteObj releaseState is true');
      throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
    }

    this.__call_obj__.onRelease(callback);
  }

  onRemoteStateChange(callback) {
    hilog.sLogI(domainID, TAG, 'Caller onRemoteStateChange jscallback called.');
    if (typeof callback !== 'function') {
      hilog.sLogI(domainID, TAG, 'Caller onRemoteStateChange ' + typeof callback);
      throw new ThrowInvalidParamError('Parameter error: Failed to get callback, must be a function.');
    }

    if (this.releaseState === true) {
      hilog.sLogI(domainID, TAG, 'Caller onRemoteStateChange remoteObj releaseState is true');
      throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
    }

    this.__call_obj__.onRemoteStateChange(callback);
  }

  on(type, callback) {
    hilog.sLogI(domainID, TAG, 'Caller onRelease jscallback called.');
    if (typeof type !== 'string' || type !== 'release') {
      hilog.sLogI(domainID, TAG,
        'Caller onRelease error, input [type] is invalid.');
      throw new ThrowInvalidParamError('Parameter error: Failed to get type, must be string type release.');
    }

    if (typeof callback !== 'function') {
      hilog.sLogI(domainID, TAG, 'Caller onRelease error ' + typeof callback);
      throw new ThrowInvalidParamError('Parameter error: Failed to get callback, must be a function.');
    }

    if (this.releaseState === true) {
      hilog.sLogI(domainID, TAG, 'Caller onRelease error, remoteObj releaseState is true');
      throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
    }

    this.__call_obj__.onRelease(callback);
  }

  off(type, callback) {
    if (typeof type !== 'string' || type !== 'release') {
      hilog.sLogI(domainID, TAG,
         'Caller onRelease error, input [type] is invalid.');
      throw new ThrowInvalidParamError('Parameter error: Failed to get type, must be string type release.');
    }

    if (callback && typeof callback !== 'function') {
      hilog.sLogI(domainID, TAG, 'Caller onRelease error ' + typeof callback);
      throw new ThrowInvalidParamError('Parameter error: Failed to get callback, must be a function.');
    }
    // Empty
  }

  callCheck(method, data) {
    if (typeof method !== 'string' || typeof data !== 'object') {
      hilog.sLogI(domainID, TAG,
        'Caller callCheck ' + typeof method + ' ' + typeof data);
      return new ThrowInvalidParamError('Parameter error: Failed to get method or data, ' +
        'method must be a string, data must be a rpc.Parcelable');
    }

    if (method === '' || data == null) {
      hilog.sLogI(domainID, TAG,
        'Caller callCheck ' + method + ', ' + data);
      return new ThrowInvalidParamError('Parameter error: method or data is empty, Please check it.');
    }

    if (this.releaseState === true) {
      hilog.sLogI(domainID, TAG, 'Caller callCheck this.callee release');
      return new BusinessError(ERROR_CODE_CALLER_RELEASED);
    }

    if (this.__call_obj__.callee == null) {
      hilog.sLogI(domainID, TAG, 'Caller callCheck this.callee is nullptr');
      return new BusinessError(ERROR_CODE_CLAAEE_INVALID);
    }
    return null;
  }

  buildMsgData(method, data) {
    let msgData = rpc.MessageSequence.create();
    msgData.writeString(method);
    msgData.writeParcelable(data);
    return msgData;
  }
}

export default Caller;
