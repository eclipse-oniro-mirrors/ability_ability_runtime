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

#include "app_mgr_service_event_handler.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
AMSEventHandler::AMSEventHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler,
    const std::weak_ptr<AppMgrServiceInner> &appMgr)
    : AAFwk::EventHandlerWrap(taskHandler), appMgr_(appMgr)
{
    TAG_LOGI(AAFwkTag::APPMGR, "instance created");
}

AMSEventHandler::~AMSEventHandler()
{
    TAG_LOGI(AAFwkTag::APPMGR, "instance destroyed");
}

void AMSEventHandler::ProcessEvent(const AAFwk::EventWrap &event)
{
    auto appManager = appMgr_.lock();
    if (!appManager) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appManager");
        return;
    }
    appManager->HandleTimeOutInner(event);
}

AppEventUtil &AppEventUtil::GetInstance()
{
    static AppEventUtil instance;
    return instance;
}

void AppEventUtil::AddEvent(std::shared_ptr<AppRunningRecord> appRecord, uint32_t eventId, int64_t param)
{
    if (appRecord == nullptr) {
        return;
    }
    std::lock_guard lock(appEventListMutex_);
    appEventList_.emplace_back(eventId, param, appRecord);
}

bool AppEventUtil::HasEvent(std::shared_ptr<AppRunningRecord> appRecord, uint32_t eventId)
{
    if (appRecord == nullptr) {
        return false;
    }
    std::lock_guard lock(appEventListMutex_);
    for (const auto &item : appEventList_) {
        if (item.appRecord.lock() == appRecord && item.eventId == eventId) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<AppRunningRecord> AppEventUtil::RemoveEvent(uint32_t eventId, int64_t param)
{
    return AAFwk::MyStatus::GetInstance().runningRecord_;
}

std::list<AppEventData> AppEventUtil::RemoveEvent(std::shared_ptr<AppRunningRecord> appRecord, uint32_t eventId)
{
    std::list<AppEventData> result;
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
