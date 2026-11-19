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

#ifndef OHOS_ABILITY_RUNTIME_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_APP_UTILS_H

#include <string>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AppUtils
 * provides app utils.
 */
class AppUtils {
public:
    static bool isSupportMultiInstance;
    static bool isStartOptionsWithAnimation;
    static bool isPrepareTerminateEnabled;

public:
    /**
     * GetInstance, get an instance of AppUtils.
     *
     * @return An instance of AppUtils.
     */
    static AppUtils &GetInstance()
    {
        static AppUtils instance;
        return instance;
    }

    /**
     * AppUtils, destructor.
     *
     */
    ~AppUtils() {}

    /**
     * IsSupportMultiInstance, check if it supports multi-instance.
     *
     * @return Whether it supports multi-instance.
     */
    bool IsSupportMultiInstance()
    {
        return isSupportMultiInstance;
    }

    /**
     * IsStartOptionsWithAnimation, check whether the start options have animation.
     *
     * @return Whether the start options have animation.
     */
    bool IsStartOptionsWithAnimation()
    {
        return isStartOptionsWithAnimation;
    }

    /**
     * IsPrepareTerminateEnabled, check if it supports prepare terminate.
     *
     * @return Whether it supports prepare terminate.
     */
    bool IsPrepareTerminateEnabled()
    {
        return isPrepareTerminateEnabled;
    }

private:
    AppUtils() {}

    DISALLOW_COPY_AND_MOVE(AppUtils);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H
