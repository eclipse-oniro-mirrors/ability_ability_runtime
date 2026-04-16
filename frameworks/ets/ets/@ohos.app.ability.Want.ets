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

import hilog from '@ohos.hilog'
import rpc from '@ohos.rpc';
import { RecordData } from '@ohos.base';

type valueType = Any;
const DOMAIN_ID = 0xD001300;
const TAG = 'RecordSerializeTool';
const int32Max: number = Math.pow(2, 31) - 1;
const int32Min: number = -Math.pow(2, 31);

const PARAM_RESV_WINDOW_MODE = 'ohos.aafwk.param.windowMode';
const PARAM_RESV_DISPLAY_ID = 'ohos.aafwk.param.displayId';

const TYPE_PROPERTY = 'type';
const REMOTE_OBJECT = 'RemoteObject';
const VALUE_PROPERTY = 'value';
const PROPERTIES_SIZE: number = 2;

interface ComplexArrayData {
  intList: Array<int>;
  boolList: Array<boolean>;
  doubleList: Array<double>;
  stringList: Array<string>;
  objectList: Array<Any>;
}

class RecordWriter {
  private buffer = new StringBuilder();
  private store = new Set<Object>();
  private wantParams: NativeWantParams | null = null;
  private static readonly GETTER_PREFIX = '<get>'

  private static asFixedArray<E>(obj: object): FixedArray<E> {
    return obj as FixedArray<E>
  }

  constructor(wantParams: long) {
    this.wantParams = new NativeWantParams(wantParams);
  }

  public write(obj: Object): boolean {
    return this.parseRecord(obj);
  }

  private parseRecord(obj: Any): boolean {
    if (obj instanceof Record) {
      this.writeRecordToNative(obj as Object as Record<string, valueType>, this.wantParams);
      return true;
    } else {
      hilog.error(DOMAIN_ID, TAG, `input object not record type`);
      return false;
    }
  }

  private writeObject(obj: Any): void {
    if (obj === null) {
      this.buffer.append('null');
    } else if (obj === undefined) {
      this.buffer.append('undefined');
    } else if (obj instanceof String) {
      this.buffer.append(JSON.stringify(obj as String));
    } else if (this.writeValueType(obj as Object)) {
      // nothing to do
    } else if (obj instanceof Array) {
      this.writeArray(obj as Object as Array<valueType>);
    } else if (obj instanceof Record) {
      this.writeRecord(obj as Object as Record<string, valueType>);
    } else if (obj instanceof Object) {
      const objCls = Class.of(obj);
      if (objCls.isFixedArray()) {
        this.writeBuildArray(RecordWriter.asFixedArray<Any>(obj));
      }
    } else {
        this.buffer.append('null');
    }
  }

  private tupleToArray(tuple: Tuple): Array<valueType> {
    let arr: Array<valueType> = [];

    const obj = tuple as Object;
    const objClass = Class.of(obj);
    const writableFields = this.getWritableFields(objClass, obj);
    for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
      const fieldValue = writableFields[fieldIdx][1];
      arr.push(fieldValue);
    }
    return arr;
  }

  private isClassType(cls: Class): boolean {
    return (!cls.isEnum()
             && !cls.isInterface()
             && !cls.isFixedArray()
             && !cls.isUnion()
             && !cls.isPrimitive())
  }

  private writeObjectToNative(key: string, obj: Any, wantParams?: NativeWantParams | null): boolean {
    let localWantParams = (wantParams === undefined) ? this.wantParams : wantParams;
    hilog.debug(DOMAIN_ID, TAG, `key: %{public}s, value type: %{public}s`, JSON.stringify(key), typeof obj);
    // skip reserved param
    if (this.blackListFilter(key)) {
      hilog.debug(DOMAIN_ID, TAG, `%{public}s is filtered.`, JSON.stringify(key));
      return true;
    }
    if (obj === null) {
      hilog.debug(DOMAIN_ID, TAG, `value is null, key: %{public}s`, JSON.stringify(key));
      return true;
    } else if (obj === undefined) {
      hilog.debug(DOMAIN_ID, TAG, `value is undefined, key: %{public}s`, JSON.stringify(key));
      return true;
    } else if (obj instanceof String) {
      localWantParams?.setParam(key, obj as String);
      return true;
    } else if (this.writeValueTypeToNative(localWantParams, key, obj as Object)) {
      // nothing to do
      return true;
    } else if (obj instanceof Array) {
      this.writeArrayToNative(key, obj as Object as Array<valueType>, localWantParams);
      return true;
    } else if (obj instanceof Tuple) {
      let tupleAsArray = this.tupleToArray(obj as Tuple);
      this.writeArrayToNative(key, tupleAsArray as Object as Array<valueType>, localWantParams);
      return true;
    } else if (this.isRemoteObjectFormat(obj)) { // consistent with napi behavior
      hilog.debug(DOMAIN_ID, TAG, `key: %{public}s, value is rpc.RemoteObject type`, JSON.stringify(key));
      let newWantParams = new NativeWantParams();
       if (this.writeRemoteObjectToNative(obj as Object as Record<string, valueType>, newWantParams) === true) {
          localWantParams?.setParam(key, newWantParams);
       }
      return true;
    } else if (this.isRemoteProxyFormat(obj)) { // consistent with napi behavior
      hilog.debug(DOMAIN_ID, TAG, `key: %{public}s, value is rpc.RemoteProxy type`, JSON.stringify(key));
      let newWantParams = new NativeWantParams();
       if (this.writeRemoteProxyToNative(obj as Object as Record<string, valueType>, newWantParams) === true) {
          localWantParams?.setParam(key, newWantParams);
       }
      return true;
    } else if (obj instanceof Record) {
      let newWantParams = new NativeWantParams();
      this.writeRecordToNative(obj as Object as Record<string, valueType>, newWantParams);
      localWantParams?.setParam(key, newWantParams);
      return true;
    } else {
      const objCls = Class.of(obj as Object);
      if (this.isClassType(objCls)) {
        // enum value are string
        if (typeof obj === 'string') {
          hilog.debug(DOMAIN_ID, TAG, `key: %{public}s,  value is Class string`, key);
          let value: string = String(obj as Object);
          localWantParams?.setParam(key, value);
          return true;
        }
        // enum value are number
        if (typeof obj === 'number') {
          hilog.debug(DOMAIN_ID, TAG, `key: %{public}s,  value is Class number`, key);
          let value: string = String(obj as Object);
          let num: number = Number(value);
          if (Number.isInteger(num)) {
            if (num >= int32Min && num <= int32Max) {
              localWantParams?.setParam(key, num as int);
            } else {
              localWantParams?.setParam(key, num as double);
            }
          } else {
            localWantParams?.setParam(key, num as double);
          }
          return true;
        }
        let newWantParams = new NativeWantParams();
        this.writeClassValueToNative(obj as Object, objCls, newWantParams);
        localWantParams?.setParam(key, newWantParams);
        return true;
      } else {
        hilog.error(DOMAIN_ID, TAG, `object type not support parse, key: %{public}s`, key);
      }
      return true;
    }
    return false;
  }

  private blackListFilter(key: string): boolean {
    if (key === PARAM_RESV_WINDOW_MODE) {
      return true;
    }
    if (key === PARAM_RESV_DISPLAY_ID) {
      return true;
    }
    return false;
  }

  private writeValueType(obj: Object): boolean {
    if (obj instanceof Boolean) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Byte) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Char) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Short) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Int) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Long) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Float) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof Double) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else if (obj instanceof BigInt) {
      this.buffer.append(JSON.stringify(obj));
      return true;
    } else {
      return false;
    }
  }

  private writeValueTypeToNative(wantParams: NativeWantParams | null, key: string, obj: Object): boolean {
    if (obj instanceof Boolean) {
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Byte) {
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Char) {
      wantParams?.setParam(key, Char.toByte(obj));
      return true;
    } else if (obj instanceof Short) {
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Int) {
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Long) {
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Float) {
      wantParams?.setParam(key, obj);
      return true;
    } else if (obj instanceof Double) {
      // consistent with napi behavior
      if (key === VALUE_PROPERTY) {
        let value: double = obj;
        if (Number.isInteger(value) && value >= int32Min && value <= int32Max) {
          wantParams?.setParam(key, value as int);
        } else {
          wantParams?.setParam(key, value as double);
        }
      } else {
        wantParams?.setParam(key, obj);
      }
      return true;
    } else if (obj instanceof BigInt) {
      // consistent with napi behavior
      return true;
    } else {
      return false;
    }
  }

  private writeArray(arr: Array<valueType>): void {
    this.buffer.append('[');
    const length = arr.length as int;
    this.checkReferencesCycle(arr);
    this.store.add(arr);
    for (let idx = 0; idx < length; idx++) {
      if (arr[idx] == null) {
        this.buffer.append('null');
      } else {
        this.writeObject(arr[idx]);
      }
      if (idx < length - 1) {
        this.buffer.append(',');
      }
    }
    this.store.delete(arr);
    this.buffer.append(']');
  }

  private unwrapArrayElementNumber(element: valueType, complexArray: ComplexArrayData): void {
    if (element instanceof Int) {
      hilog.debug(DOMAIN_ID, TAG, `array value type Int`);
      if (complexArray.doubleList.length > 0) {
        complexArray.intList.forEach(num => {
          complexArray.doubleList.push(Number(num));
        });
        complexArray.intList = [];
        complexArray.doubleList.push((element as number));
      } else {
        complexArray.intList.push((element as int));
      }
    } else if (element instanceof Long) {
      hilog.debug(DOMAIN_ID, TAG, `array value type Long`);
      // consistent with napi behavior
      const doubleValue = Number(element);
      if (complexArray.intList.length > 0) {
        complexArray.intList.forEach(num => {
          complexArray.doubleList.push(Number(num));
        });
        complexArray.intList = [];
      }
      complexArray.doubleList.push(doubleValue);
    } else if (element instanceof Double) {
      hilog.debug(DOMAIN_ID, TAG, `array value type Double`);
      if (complexArray.intList.length > 0) {
        complexArray.intList.forEach(num => {
          complexArray.doubleList.push(Number(num));
        });
        complexArray.intList = [];
      }
      complexArray.doubleList.push((element as number));
    } else {
      const objCls = Class.of(element as Object);
      // enum value are number
      if (this.isClassType(objCls)) {
        let value: string = String(element as Object);
        let enumNum: number = Number(value);
        if (Number.isInteger(enumNum) && enumNum >= int32Min && enumNum <= int32Max) {
          hilog.debug(DOMAIN_ID, TAG, `array value type Class Int`);
          if (complexArray.doubleList.length > 0) {
            complexArray.intList.forEach(num => {
              complexArray.doubleList.push(Number(num));
            });
            complexArray.intList = [];
            complexArray.doubleList.push(enumNum as double);
          } else {
            complexArray.intList.push(enumNum as int);
          }
        } else {
          hilog.debug(DOMAIN_ID, TAG, `array value type Class double`);
          if (complexArray.intList.length > 0) {
            complexArray.intList.forEach(num => {
              complexArray.doubleList.push(Number(num));
            });
            complexArray.intList = [];
          }
          complexArray.doubleList.push(enumNum as double);
        }
      }
    }
  }

  private unwrapArray(arr: Array<valueType>, complexArray: ComplexArrayData): void {
    const length = arr.length as int;
    this.checkReferencesCycle(arr);
    this.store.add(arr);
    for (let idx = 0; idx < length; idx++) {
      const element = arr[idx];
      if (element === null || element === undefined) {
        continue;
      }
      const valueType = typeof element;
      hilog.debug(DOMAIN_ID, TAG, `array value type: %{public}s`, valueType);
      switch (valueType) {
        case 'string':
          if (element instanceof String) {
            complexArray.stringList.push(element as string);
          } else {
            const objCls = Class.of(element as Object);
            if (this.isClassType(objCls)) {
              let value: string = String(element as Object);
              complexArray.stringList.push(value as string);
            }
          }
          break;
        case 'boolean':
          complexArray.boolList.push((element as Boolean));
          break;
        case 'number':
        case 'int':
        case 'long':
        case 'float':
          try {
            this.unwrapArrayElementNumber(element, complexArray);
          } catch (err) {
            hilog.error(DOMAIN_ID, TAG, `unwrapArray number error: ${err}`);
          }
          break;
        case 'object':
          complexArray.objectList.push(element);
          break;
        default:
          hilog.error(DOMAIN_ID, TAG, `unknown array value type: %{public}s`, JSON.stringify(arr[idx]));
          break;
      }
    }
    this.store.delete(arr);
  }

  private handleArrayObject(obj: object, wantParams: NativeWantParams): boolean {
    if (Array.isArray(obj)) { // consistent with napi behavior
      let localArr: Array<valueType> = obj as Array<valueType>;
      hilog.debug(DOMAIN_ID, TAG, `array object element type array, length: %{public}d`, localArr.length);
      for (let i = 0; i < localArr.length; i++) {
        this.writeObjectToNative(i.toString(), localArr[i] as Object, wantParams);
      }
      wantParams.setParam("length", localArr.length as int);
    } else if (obj instanceof Tuple) { // consistent with napi behavior
      let tupleAsArray = this.tupleToArray(obj as Tuple);
      hilog.debug(DOMAIN_ID, TAG, `array object element type tuple, length: %{public}d`, tupleAsArray.length);
      for (let i = 0; i < tupleAsArray.length; i++) {
        this.writeObjectToNative(i.toString(), tupleAsArray[i] as Object, wantParams);
      }
      wantParams.setParam("length", tupleAsArray.length as int);
    } else if (obj instanceof Record) {
      hilog.debug(DOMAIN_ID, TAG, `array object element type Record`);
      this.writeRecordToNative(obj as Object as Record<string, valueType>, wantParams);
    } else {
      const objCls = Class.of(obj);
      if (this.isClassType(objCls)) {
        hilog.debug(DOMAIN_ID, TAG, `array object element type class`);
        this.writeClassValueToNative(obj, objCls, wantParams);
      } else {
        hilog.error(DOMAIN_ID, TAG, `array member object type invalid`);
        return false;
      }
    }
    return true;
  }

  private writeArrayToNative(key: string, arr: Array<valueType>, wantParams: NativeWantParams | null): void {
    let complexArray: ComplexArrayData = {
      intList: [],
      boolList: [],
      doubleList: [],
      stringList: [],
      objectList: []
    }
    this.unwrapArray(arr, complexArray);
    if (complexArray.objectList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s objectList size: %{public}d`, key, complexArray.objectList.length);
      let wantParamsArray: Array<NativeWantParams> = [];
      let newWantParams: NativeWantParams;
      for (let i = 0; i < complexArray.objectList.length; i++) {
        newWantParams = new NativeWantParams();
        if (this.handleArrayObject(complexArray.objectList[i] as object, newWantParams) != true) {
          hilog.error(DOMAIN_ID, TAG, `parse array member failed, index: %{public}d`, i);
        }
        wantParamsArray.push(newWantParams);
      }
      wantParams?.SetArrayWantParams(key, wantParamsArray);
    } else if (complexArray.stringList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s stringList size: %{public}d`, key, complexArray.stringList.length);
      wantParams?.SetArrayStringParam(key, complexArray.stringList);
    } else if (complexArray.intList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s intList size: %{public}d`, key, complexArray.intList.length);
      wantParams?.SetArrayIntParam(key, complexArray.intList);
    } else if (complexArray.boolList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s boolList size: %{public}d`, key, complexArray.boolList.length);
      wantParams?.SetArrayBooleanParam(key, complexArray.boolList);
    } else if (complexArray.doubleList.length > 0) {
      hilog.debug(DOMAIN_ID, TAG, `array %{public}s doubleList size: %{public}d`, key, complexArray.doubleList.length);
      wantParams?.SetArrayDoubleParam(key, complexArray.doubleList);
    }
  }

  private writeBuildArray<E>(array: FixedArray<E>): void {
    this.buffer.append('[');
    const length = array.length;
    this.checkReferencesCycle(array);
    this.store.add(array);
    for (let idx = 0; idx < length; idx++) {
      let member = array[idx];
      if (member == null) {
        this.buffer.append('null');
      } else {
        this.writeObject(member);
      }
      if (idx < length - 1) {
        this.buffer.append(',');
      }
    }
    this.store.delete(array);
    this.buffer.append(']');
  }

  private writeRecord(rec: Record<string, valueType>): void {
    this.buffer.append('{');
    this.checkReferencesCycle(rec);
    this.store.add(rec);
    let isFirst = true;
    for (let key of rec.keys()) {
      if (rec[key] !== undefined) {
        if (!isFirst) {
          this.buffer.append(',');
        } else {
          isFirst = false;
        }
        this.buffer.append(JSON.stringify(key as String));
        this.buffer.append(':');
        this.writeObject(rec[key]);
      }
    }
    this.store.delete(rec);
    this.buffer.append('}');
  }

  private isRemoteObjectFormat(obj: valueType): boolean {
    if (!(obj instanceof Record)) {
      return false;
    }
    let hasType = false;
    let hasValue = false;
    let typeValue: valueType = undefined;
    let valueValue: valueType = undefined;
    let count = 0;
    let rec: Record<string, valueType> = obj as Object as Record<string, valueType>;
    for (let key of rec.keys()) {
      if (rec[key] === undefined || rec[key] === null) {
        return false;
      }
      if (key === TYPE_PROPERTY) {
        hasType = true;
        typeValue = rec[key];
      } else if (key === VALUE_PROPERTY) {
        hasValue = true;
        valueValue = rec[key];
      }
      count++;
    }

    if (count !== PROPERTIES_SIZE) {
      return false;
    }
    if (!hasType || !hasValue) {
      return false;
    }
    if (typeof typeValue !== 'string' || typeValue !== REMOTE_OBJECT) {
      return false;
    }
    if (!(valueValue instanceof rpc.RemoteObject)) {
      return false;
    }

    return true;
  }

  private isRemoteProxyFormat(obj: valueType): boolean {
    if (!(obj instanceof Record)) {
      return false;
    }
    let hasType = false;
    let hasValue = false;
    let typeValue: valueType = undefined;
    let valueValue: valueType = undefined;
    let count = 0;
    let rec: Record<string, valueType> = obj as Object as Record<string, valueType>;
    for (let key of rec.keys()) {
      if (rec[key] === undefined || rec[key] === null) {
        return false;
      }
      if (key === TYPE_PROPERTY) {
        hasType = true;
        typeValue = rec[key];
      } else if (key === VALUE_PROPERTY) {
        hasValue = true;
        valueValue = rec[key];
      }
      count++;
    }

    if (count !== PROPERTIES_SIZE) {
      return false;
    }
    if (!hasType || !hasValue) {
      return false;
    }
    if (typeof typeValue !== 'string' || typeValue !== REMOTE_OBJECT) {
      return false;
    }
    if (!(valueValue instanceof rpc.RemoteProxy)) {
      return false;
    }

    return true;
  }

  private writeRemoteObjectToNative(obj: Record<string, valueType>, wantParams: NativeWantParams | null): boolean {
    for (let key of obj.keys()) {
      if (key === TYPE_PROPERTY) {
        wantParams?.setParam(TYPE_PROPERTY, REMOTE_OBJECT);
      } else if (key === VALUE_PROPERTY) {
        if (wantParams === null || wantParams.setParam(VALUE_PROPERTY, obj[key] as rpc.RemoteObject) != true) {
          hilog.error(DOMAIN_ID, TAG, `write RemoteObject to native failed`);
          return false;
        }
      }
    }
    return true;
  }

  private writeRemoteProxyToNative(obj: Record<string, valueType>, wantParams: NativeWantParams | null): boolean {
    for (let key of obj.keys()) {
      if (key === TYPE_PROPERTY) {
        wantParams?.setParam(TYPE_PROPERTY, REMOTE_OBJECT);
      } else if (key === VALUE_PROPERTY) {
        if (wantParams === null || wantParams.setParam(VALUE_PROPERTY, obj[key] as rpc.RemoteProxy) != true) {
          hilog.error(DOMAIN_ID, TAG, `write RemoteProxy to native failed`);
          return false;
        }
      }
    }
    return true;
  }

  private writeRecordToNative(rec: Record<string, valueType>, wantParams: NativeWantParams | null): boolean {
    this.checkReferencesCycle(rec);
    this.store.add(rec);
    let isSuccess = false;
    for (let key of rec.keys()) {
      if (rec[key] === undefined || rec[key] === null) {
        continue;
      }
      if (this.writeObjectToNative(key as String, rec[key] as Object, wantParams) != true) {
        hilog.error(DOMAIN_ID, TAG, `write object to native failed, key: %{public}s`, JSON.stringify(key as String));
      }
      isSuccess = true;
    }
    this.store.delete(rec);
    if (isSuccess != true) {
      hilog.info(DOMAIN_ID, TAG, `null record`);
    }
    return isSuccess;
  }

  private writeClassValueToNative(obj: Object, objCls: Class, wantParams: NativeWantParams): boolean {
    const writableFields = this.getWritableFields(objCls, obj);

    let isSuccess = false;
    if (writableFields.length > 0) {
      this.checkReferencesCycle(obj);
      this.store.add(obj);
      isSuccess = this.writeClassFields(writableFields, wantParams);
      this.store.delete(obj);
    }

    return isSuccess;
  }

  private writeField(fieldName: string, fieldVal: Any, wantParams: NativeWantParams): boolean {
    if (fieldVal === undefined) {
      return false;
    }

    const fieldValType = Class.ofAny(fieldVal);
    if (fieldValType instanceof Function) {
      return false;
    }

    return this.writeObjectToNative(fieldName, fieldVal, wantParams);
  }

  private writeClassFields(writableFields: Array<[string, Any]>, wantParams: NativeWantParams): boolean {
    const FIELD_NAME = 0
    const FIELD_VALUE = 1

    let isSuccess = false;
    for (let fieldIdx = 0; fieldIdx < writableFields.length; fieldIdx++) {
      const fieldTypeValuePair = writableFields[fieldIdx];

      const objFieldName = fieldTypeValuePair[FIELD_NAME];
      const objFieldValue = fieldTypeValuePair[FIELD_VALUE];

      if (this.writeField(objFieldName, objFieldValue, wantParams) == true) {
        isSuccess= true;
      }
    }

    return isSuccess;
  }

  private findKeyIndex(fieldName: string, fields: Array<[string, Any]>): number {
    return fields.findIndex((writableField) => writableField[0]  == fieldName);
  }

  private addWritableProperty(cls: Class, obj: object, propName: string, writableFields: Array<[string, Any]>): void {
      const getterName = RecordWriter.GETTER_PREFIX + propName
      const getter = cls.getInstanceMethod(getterName)
      if (getter === undefined) {
          return
      }

      const propVal = getter.invoke(obj)

      const propNameValuePair: [string, Any] = [propName, propVal]
      writableFields.push(propNameValuePair)
  }

  private addWritableProperties(cls: Class, obj: object, writableFields: Array<[string, Any]>): void {
      for (const method of cls.getInstanceMethods()) {
          const methodName = method.getName()
          if (methodName.startsWith(RecordWriter.GETTER_PREFIX)) {
              const propName = methodName.substring(RecordWriter.GETTER_PREFIX.length)
              const propValue = method.invoke(obj)
              const fieldNameValuePair: [string, Any] = [propName, propValue]
              writableFields.push(fieldNameValuePair)
          }
      }
  }

  private getWritableFields(cls: Class, obj: object): Array<[string, Any]> {
    const writableFields = new Array<[string, Any]>();
    let fieldRename : string | undefined = undefined;

    for (const field of cls.getInstanceFields()) {
      const index: int = this.findKeyIndex(field.getName(), writableFields).toInt();
      const fieldNameValuePair: [string, Any] = [field.getName(), field.getValue(obj)];
      if (fieldRename != undefined || index == -1) {
        writableFields.push(fieldNameValuePair);
      } else {
        writableFields[index] = fieldNameValuePair;
      }
      if (reflect.isLiteralInitializedInterface(obj)) {
        this.addWritableProperties(cls, obj, writableFields)
      }
    }

    return writableFields;
  }

  private checkReferencesCycle(obj: Object): void {
    if (this.store.has(obj)) {
      throw new TypeError('cyclic object value');
    }
  }
}

export class RecordSerializeTool {
  public static unwrapRecordNoThrow(obj: Record<string, Object>, nativeWantParams: long): boolean {
    try {
      return RecordSerializeTool.unwrapRecord(obj as Object as Record<string, Any>, nativeWantParams);
    } catch (err) {
      hilog.error(DOMAIN_ID, TAG, `RecordSerializeTool.stringify error: ${err}`);
      return false;
    }
  }

  public static parseNoThrow(text: string): Record<string, Object> {
    try {
      return RecordSerializeTool.parse(text) as Object as Record<string, Object>;
    } catch (err) {
      hilog.error(DOMAIN_ID, TAG, `RecordSerializeTool.parse error: ${err}`);
      return new Record<string, Object>();
    }
  }

  public static unwrapRecord(obj: Record<string, valueType>, nativeWantParams: long): boolean {
    return new RecordWriter(nativeWantParams).write(obj);
  }

  public static parse(text: string): Record<string, valueType> {
    let jsonValue = JSON.parseJsonElement(text);
    let res = RecordSerializeTool.jsonValue2Object(jsonValue);
    if (!(res instanceof Record)) {
      throw new TypeError('RecordSerializeTool parse only used for Record');
    }
    return res as Record<string, valueType>;
  }

  private static jsonValue2Object(value: jsonx.JsonElement): string | number | boolean | null |
    Array<valueType> | Record<string, valueType> {
    let vType = value.jsonType;
    if (vType === jsonx.JsonType.JsonString) {
      return value.asString();
    } else if (vType === jsonx.JsonType.JsonNumber) {
      return value.asDouble();
    } else if (vType === jsonx.JsonType.JsonTrue) {
      return new Boolean(true);
    } else if (vType === jsonx.JsonType.JsonFalse) {
      return new Boolean(false);
    } else if (vType === jsonx.JsonType.JsonNull) {
      return null;
    } else if (vType === jsonx.JsonType.JsonArray) {
      let values: Array<jsonx.JsonElement> = value.asArray();
      let result: Array<valueType> = new Array<valueType>();
      for (let i: int = 0; i < values.length; i++) {
        result.push(RecordSerializeTool.jsonValue2Object(values[i]));
      }
      return result;
    } else if (vType === jsonx.JsonType.JsonObject) {
      let iterVal: IterableIterator<[string, jsonx.JsonElement]> = value.$_iterator();
      let result: Record<string, valueType> = new Record<string, valueType>();
      for (let t of iterVal) {
        result[t[0]] = RecordSerializeTool.jsonValue2Object(t[1]);
      }
      return result;
    } else {
      throw new TypeError('unknown JSONElement type');
    }
  }
}

class NativeWantParamsCleaner {
  public nativeWantParams: long = 0;

  constructor(nativeWantParams: long) {
    this.nativeWantParams = nativeWantParams;
  }

  private static native nativeDestroy(nativeWantParams: long): void;

  public clean(): void {
    NativeWantParamsCleaner.nativeDestroy(this.nativeWantParams);
    this.nativeWantParams = 0;
  }
}

function nativeWantParamsCleanerCallback(cleaner: NativeWantParamsCleaner): void {
  cleaner.clean();
}

let destroyRegister = new FinalizationRegistry<NativeWantParamsCleaner>(nativeWantParamsCleanerCallback);

class NativeWantParams {
  static {
    loadLibrary("want_ani_kit.z");
  }

  private nativeWantParams: long = 0;
  private cleaner: NativeWantParamsCleaner | null = null;

  constructor(wantParams?: long) {
    if (wantParams === undefined) {
      this.nativeWantParams = NativeWantParams.nativeCreate();
      this.registerCleaner(this.nativeWantParams)
      return;
    }
    this.nativeWantParams = wantParams;
  }

  private static native nativeCreate(): long;

  private static native nativeSetStringParam(nativeWantParams: long, key: string, value: string): boolean;
  private static native nativeSetDoubleParam(nativeWantParams: long, key: string, value: double): boolean;
  private static native nativeSetIntParam(nativeWantParams: long, key: string, value: int): boolean;
  private static native nativeSetLongParam(nativeWantParams: long, key: string, value: long): boolean;
  private static native nativeSetBooleanParam(nativeWantParams: long, key: string, value: boolean): boolean;
  private static native nativeSetWantParams(nativeWantParams: long, key: string, value: long): boolean;

  private static native nativeSetArrayStringParam(nativeWantParams: long, key: string, value: Array<string>): boolean;
  private static native nativeSetArrayDoubleParam(nativeWantParams: long, key: string, value: Array<double>): boolean;
  private static native nativeSetArrayIntParam(nativeWantParams: long, key: string, value: Array<int>): boolean;
  private static native nativeSetArrayLongParam(nativeWantParams: long, key: string, value: Array<long>): boolean;
  private static native nativeSetArrayBooleanParam(nativeWantParams: long, key: string, value: Array<boolean>): boolean;
  private static native nativeSetArrayWantParams(nativeWantParams: long, key: string, value: Array<long>): boolean;

  private static native nativeSetRemoteObjectParam(
    nativeWantParams: long, key: string, value: rpc.RemoteObject): boolean;
  private static native nativeSetRemoteProxyParam(
    nativeWantParams: long, key: string, value: rpc.RemoteProxy): boolean;

  private registerCleaner(nativeWantParams: long): void {
    this.cleaner = new NativeWantParamsCleaner(nativeWantParams);
    destroyRegister.register(this, this.cleaner!, this);
  }

  public setParam(key: string, value: string): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetStringParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: int): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetIntParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: long): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetLongParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: double): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetDoubleParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: boolean): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetBooleanParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: NativeWantParams): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetWantParams(this.nativeWantParams, key, value.nativeWantParams);
  }

  public SetArrayStringParam(key: string, value: Array<string>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayStringParam(this.nativeWantParams, key, value);
  }

  public SetArrayDoubleParam(key: string, value: Array<double>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayDoubleParam(this.nativeWantParams, key, value);
  }

  public SetArrayIntParam(key: string, value: Array<int>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayIntParam(this.nativeWantParams, key, value);
  }

  public SetArrayLongParam(key: string, value: Array<long>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayLongParam(this.nativeWantParams, key, value);
  }

  public SetArrayBooleanParam(key: string, value: Array<boolean>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetArrayBooleanParam(this.nativeWantParams, key, value);
  }

  public SetArrayWantParams(key: string, value: Array<NativeWantParams>): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    let nativeWantParams: Array<long> = new Array<long>();
    for (let item of value) {
      nativeWantParams.push(item.nativeWantParams);
    }
    return NativeWantParams.nativeSetArrayWantParams(this.nativeWantParams, key, nativeWantParams);
  }

  public setParam(key: string, value: rpc.RemoteObject): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetRemoteObjectParam(this.nativeWantParams, key, value);
  }

  public setParam(key: string, value: rpc.RemoteProxy): boolean {
    if (this.nativeWantParams == 0) {
      hilog.error(DOMAIN_ID, TAG, `nativeWantParams null`);
      return false;
    }
    return NativeWantParams.nativeSetRemoteProxyParam(this.nativeWantParams, key, value);
  }
}

export default class Want {
  bundleName?: string;
  abilityName?: string;
  deviceId?: string;
  uri?: string;
  type?: string;
  flags?: int;
  action?: string;
  parameters?: Record<string, RecordData>;
  entities?: Array<string>;
  moduleName?: string;
  readonly fds?: Record<string, int>;
}
