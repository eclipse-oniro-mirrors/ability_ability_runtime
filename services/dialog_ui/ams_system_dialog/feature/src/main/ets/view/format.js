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

export function format(obj) {
    let nestedObj = {};
    for (const key in obj) {
        if (key.includes('.')) {
            const parts = key.split('.');
            if (key.includes('.array')) {
                processArrayKey(nestedObj, parts, obj[key]);
            } else {
                processObjectKey(nestedObj, parts, obj[key]);
            }
        } else {
            nestedObj[key] = obj[key];
        }
    }
    nestedObj = formatFilterArrNull(nestedObj);
    return nestedObj;
}

function processObjectKey(nestedObj, parts, value) {
    let current = nestedObj;
    for (let i = 0; i < parts.length - 1; i++) {
        const part = parts[i];
        if (!current[part] || typeof current[part] !== 'object') {
            current[part] = {};
        }
        current = current[part];
    }
    current[parts[parts.length - 1]] = value;
}

function processArrayKey(nestedObj, parts, value) {
    const index = parseInt(parts[1].substring(5));
    let current = nestedObj;
    for (let i = 0; i < parts.length - 2; i++) {
        const part = parts[i];
        if (!current[part] || !Array.isArray(current[part])) {
            current[part] = [];
        }
        current = current[part];
    }

    if (parts[parts.length - 1] === 'items') {
        current[index] = value;
        return;
    }

    if (!current[index] || typeof current[index] !== 'object') {
        current[index] = {};
    }
    current[index][parts[parts.length - 1]] = value;
}

function formatFilterArrNull(nestedObj) {
    Object.keys(nestedObj).forEach((key) => {
        if (Array.isArray(nestedObj[key])) {
            nestedObj[key] = nestedObj[key].filter(item => item !== null);
        }
    });
    return nestedObj;
}

export function jsClone(objArr) {
    return JSON.parse(JSON.stringify(objArr));
}

export function addArrayParams(fieldSubProperties, fieldName, countArr) {
    let len = countArr[countArr.length - 1];

    const arr = jsClone(fieldSubProperties[0]);
    arr.forEach((item, index) => {
        const path = item.key.split('.');
        arr[index].key = `${arr[index].fatherKey}.array${len}.${path[2]}`;
        arr[index].index = len;
    });
    fieldSubProperties.push(arr);
    return fieldSubProperties;
}

export function removeArrayParams(fieldSubProperties, fieldName, arrIndex) {
    const arr = jsClone(fieldSubProperties);
    const newArr = arr.filter((item, index) => index !== arrIndex);
    return newArr;
}

export function removeFormData(formData, fieldName, arrIndex) {
    Object.keys(formData).forEach((name) => {
        if (name.includes(`${fieldName}.array${arrIndex}.`)) {
            delete formData[name];
        }
    });
    return jsClone(formData);
}