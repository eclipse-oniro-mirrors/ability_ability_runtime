/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_ENV_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_ENV_H

#include <cstdint>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Obtains the bundle name of the application
 * @param -
 * @return Returns the pointer to the bundle name if the operation is successful; returns a null pointer otherwise.
 */
const char *GetBundleName();

/**
 * @brief Obtains the source code path of this application.
 * @param -
 * @return Returns the pointer to the source code path of this application.
 */
const char *GetSrcPath();

/**
 * @brief Obtains the data path of this application.
 * @param -
 * @return Returns the pointer to the data path of this application.
 */
const char *GetDataPath();

/**
* @brief Obtains the app preload type of the application
* @param -
* @return Returns the application preload type identifier
*/
int32_t GetAppPreload();

/**
* @brief Clears the application preload state
*/
void ClearAppPreload();

/**
* @brief Invoke the AppFreeze filter callback registered by AppKit to check whether
*        a heap dump is allowed for the current process when an OOM event occurs.
*        Used by JSVM/V8 engines which load libappkit_native.z.so via dlopen.
*
* @param pid Process id of the OOM-triggering process.
* @param needDecreaseQuota Non-zero if the dump quota should be decreased, 0 otherwise.
* @param eventConfigKey Input/output buffer holding the event config key (xattr name).
*        On input it selects which engine's config to read (e.g.
*        "user.event_config.jsvm_heap_log_enable"); on output it is overwritten with
*        the resolved config value when the callback returns 1. Cannot be null.
* @param keyBufSize Size of the eventConfigKey buffer in bytes. Must be > 0.
* @return true if dump is allowed, false if not allowed.
*/
bool InvokeAppFreezeFilterCallback(int32_t pid, int32_t needDecreaseQuota,
                                   char *eventConfigKey, int32_t keyBufSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_ENV_H
