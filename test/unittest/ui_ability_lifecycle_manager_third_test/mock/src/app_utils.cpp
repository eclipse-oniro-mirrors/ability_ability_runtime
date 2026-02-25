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

#include "app_utils.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool AppUtils::isStartOptionsWithAnimation_ = false;
bool AppUtils::isStartSpecifiedProcess_ = false;
bool AppUtils::isInOnNewProcessEnableList_ = false;

AppUtils::~AppUtils() {}

AppUtils::AppUtils() {}

AppUtils &AppUtils::GetInstance()
{
    static AppUtils utils;
    return utils;
}

bool AppUtils::IsStartSpecifiedProcess()
{
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", AppUtils::isStartSpecifiedProcess_);
    return AppUtils::isStartSpecifiedProcess_;
}

bool AppUtils::IsStartOptionsWithProcessOptions()
{
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", AppUtils::isStartOptionsWithAnimation_);
    return AppUtils::isStartOptionsWithAnimation_;
}

bool AppUtils::IsStartOptionsWithAnimation()
{
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", AppUtils::isStartOptionsWithAnimation_);
    return AppUtils::isStartOptionsWithAnimation_;
}

bool AppUtils::InOnNewProcessEnableList(const std::string &bundleName)
{
    return isInOnNewProcessEnableList_;
}

bool AppUtils::IsProductAppbootSettingEnabled()
{
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
