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

#include "application_env.h"
#include "application_env_impl.h"

#include <cstring>
#include "application_context.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Obtains the bundle name of the application
 * @param -
 * @return Returns the pointer to the bundle name if the operation is successful; returns a null pointer otherwise.
 */
const char *GetBundleName()
{
    OHOS::AppExecFwk::ApplicationEnvImpl *pApplicationEnvIml = OHOS::AppExecFwk::ApplicationEnvImpl::GetInstance();
    const char *pBundleName = nullptr;

    if (pApplicationEnvIml) {
        pBundleName = pApplicationEnvIml->GetBundleName().c_str();
    }

    return pBundleName;
}

/**
 * @brief Obtains the source code path of this application.
 * @param -
 * @return Returns the pointer to the source code path of this application.
 */
const char *GetSrcPath()
{
    OHOS::AppExecFwk::ApplicationEnvImpl *pApplicationEnvIml = OHOS::AppExecFwk::ApplicationEnvImpl::GetInstance();
    const char *pSrcPath = nullptr;

    if (pApplicationEnvIml) {
        pSrcPath = pApplicationEnvIml->GetSrcPath().c_str();
    }

    return pSrcPath;
}

/**
 * @brief Obtains the data path of this application.
 * @param -
 * @return Returns the pointer to the data path of this application.
 */
const char *GetDataPath()
{
    OHOS::AppExecFwk::ApplicationEnvImpl *pApplicationEnvIml = OHOS::AppExecFwk::ApplicationEnvImpl::GetInstance();
    const char *pDataPath = nullptr;

    if (pApplicationEnvIml) {
        pDataPath = pApplicationEnvIml->GetDataPath().c_str();
    }

    return pDataPath;
}

int32_t GetAppPreload()
{
    OHOS::AppExecFwk::ApplicationEnvImpl *pApplicationEnvIml = OHOS::AppExecFwk::ApplicationEnvImpl::GetInstance();
    if (pApplicationEnvIml) {
        return pApplicationEnvIml->GetAppPreloadType();
    }
    return 0;
}

void ClearAppPreload()
{
    OHOS::AppExecFwk::ApplicationEnvImpl *pApplicationEnvIml = OHOS::AppExecFwk::ApplicationEnvImpl::GetInstance();
    if (pApplicationEnvIml) {
        pApplicationEnvIml->ClearAppPreloadType();
    }
}

bool InvokeAppFreezeFilterCallback(int32_t pid, int32_t needDecreaseQuota,
                                   char *eventConfigKey, int32_t keyBufSize)
{
    if (eventConfigKey == nullptr || keyBufSize <= 0) {
        return false;
    }
    auto applicationContext = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        return false;
    }
    auto callback = applicationContext->GetAppFreezeFilterCallback();
    if (!callback) {
        return false;
    }
    std::string key(eventConfigKey);
    bool allowed = callback(pid, needDecreaseQuota != 0, key);
    if (allowed) {
        // On success the callback writes the resolved config value back into key.
        // Copy it out to the caller's buffer (truncated if too long).
        int32_t copyLen = static_cast<int32_t>(key.size());
        if (copyLen >= keyBufSize) {
            copyLen = keyBufSize - 1;
        }
        errno_t ret = memcpy_s(eventConfigKey, keyBufSize, key.c_str(), copyLen);
        if (ret != EOK) {
            // The callback has already executed business logic (consumed dump
            // quota, set AppFreezeFilter). Returning false would mislead the
            // caller into thinking dump was not allowed while quota is already
            // consumed. Return true to reflect the actual state, and leave the
            // buffer as an empty string so the caller gets a valid (empty) key.
            TAG_LOGE(AAFwkTag::APPKIT, "memcpy_s failed, ret=%d", ret);
            eventConfigKey[0] = '\0';
            return true;
        }
        eventConfigKey[copyLen] = '\0';
    }
    return allowed;
}

#ifdef __cplusplus
}  // extern "C"
#endif
