/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "application_impl.h"

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS

namespace OHOS {
namespace AppExecFwk {
ApplicationImpl::ApplicationImpl() : curState_(APP_STATE_CREATE), recordId_(0)
{}

/**
 * @brief Set the application to the ApplicationImpl.
 *
 * @param application The application which the mainthread launched.
 *
 */
void ApplicationImpl::SetApplication(const std::shared_ptr<OHOSApplication> &application)
{
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    this->application_ = application;
}

/**
 * @brief Schedule the application to the APP_STATE_READY state.
 *
 * @return Returns true if performAppReady is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformAppReady()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return false;
    }
    application_->CleanUselessTempData();
    if (curState_ == APP_STATE_CREATE) {
        application_->OnStart();
        curState_ = APP_STATE_READY;
        return true;
    }
    TAG_LOGW(AAFwkTag::APPKIT, "curState: %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_FOREGROUND state.
 *
 * @return Returns true if PerformForeground is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformForeground()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (((curState_ == APP_STATE_READY) || (curState_ == APP_STATE_BACKGROUND)) && application_ != nullptr) {
        application_->OnForeground();
        application_->CleanUselessTempData();
        curState_ = APP_STATE_FOREGROUND;
        return true;
    }
    TAG_LOGW(AAFwkTag::APPKIT, "curState: %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_BACKGROUND state.
 *
 * @return Returns true if PerformBackground is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformBackground()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (curState_ == APP_STATE_FOREGROUND && application_ != nullptr) {
        application_->OnBackground();
        curState_ = APP_STATE_BACKGROUND;
        return true;
    }
    TAG_LOGW(AAFwkTag::APPKIT, "curState: %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_TERMINATED state.
 *
 * @param isLastProcess When it is the last application process, pass in true.
 *
 * @return Returns true if PerformTerminate is scheduled successfully;
 *         Returns false otherwise.
 */
bool ApplicationImpl::PerformTerminate(bool isLastProcess)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return false;
    }

    application_->CleanAppTempData(isLastProcess);
    if (curState_ == APP_STATE_READY || curState_ == APP_STATE_BACKGROUND) {
        application_->OnTerminate();
        curState_ = APP_STATE_TERMINATED;
        return true;
    }
    TAG_LOGW(AAFwkTag::APPKIT, "curState: %{public}d", curState_);
    return false;
}

/**
 * @brief Schedule the application to the APP_STATE_TERMINATED state.
 *
 * @return Returns true if PerformTerminate is scheduled successfully;
 *         Returns false otherwise.
 */
void ApplicationImpl::PerformTerminateStrong()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    application_->OnTerminate();
}

/**
 * @brief System determines to trim the memory.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void ApplicationImpl::PerformMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (application_ != nullptr) {
        application_->OnMemoryLevel(level);
    }
}

/**
 * @brief System determines to send the new config to application.
 *
 * @param config Indicates the updated configuration information.
 *
 */
void ApplicationImpl::PerformConfigurationUpdated(const Configuration &config, ConfigUpdateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (application_ != nullptr) {
        application_->OnConfigurationUpdated(config, AbilityRuntime::SetLevel::System, reason);
    }
}

/**
 * @brief Set the target state to application.
 *
 * @param state The target state of application.
 *
 */
int ApplicationImpl::SetState(int state)
{
    curState_ = state;
    return curState_;
}

/**
 * @brief Get the current state of application.
 *
 * @return Returns the current state of application.
 *
 */
int ApplicationImpl::GetState() const
{
    return curState_;
}

/**
 * @brief Set the RecordId to application.
 *
 * @param id recordId.
 *
 */
void ApplicationImpl::SetRecordId(int id)
{
    recordId_ = id;
}

/**
 * @brief Get the recordId of application.
 *
 * @return Returns the recordId of application.
 *
 */
int ApplicationImpl::GetRecordId() const
{
    return recordId_;
}

bool ApplicationImpl::IsUpdated() const
{
    return isUpdated_;
}

void ApplicationImpl::MarkUpdated()
{
    isUpdated_ = true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
