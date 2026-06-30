/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "uri_permission_manager_service.h"

#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "dynamic_feature_manager.h"
#include "feature/idynamic_feature.h"

namespace OHOS {
namespace AAFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<UriPermissionManagerService>::GetInstance().get());

UriPermissionManagerService::UriPermissionManagerService() : SystemAbility(URI_PERMISSION_MGR_SERVICE_ID, true) {}

UriPermissionManagerService::~UriPermissionManagerService()
{
    if (impl_ != nullptr) {
        impl_ = nullptr;
    }
}

void UriPermissionManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "start");
    if (!Init()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "init failed");
        return;
    }

    if (!registerToService_) {
        auto systemAabilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAabilityMgr || systemAabilityMgr->AddSystemAbility(URI_PERMISSION_MGR_SERVICE_ID, impl_) != 0) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "register to systemAbilityMgr failed");
            return;
        }
        TAG_LOGI(AAFwkTag::URIPERMMGR, "register to systemAbilityMgr success");
        registerToService_ = true;
    }
}

void UriPermissionManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "call");
    SelfClean();
}

bool UriPermissionManagerService::IsServiceReady() const
{
    return ready_;
}

bool UriPermissionManagerService::Init()
{
    if (ready_) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "repeat init");
        return true;
    }

    if (impl_ == nullptr) {
        impl_ = new UriPermissionManagerStubImpl();
    }
#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    // Register dynamically-loadable dependency plugins (design ADR-1).
    DynamicFeatureManager::GetInstance().Register(FeatureId::MEDIA, "libupms_media_ext.z.so");
#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    ready_ = true;
    return true;
}

void UriPermissionManagerService::SelfClean()
{
    if (ready_) {
        ready_ = false;
        if (registerToService_) {
            registerToService_ = false;
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
