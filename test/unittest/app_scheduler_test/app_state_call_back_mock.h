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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_STATE_CALL_BACK_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_STATE_CALL_BACK_MOCK_H

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "app_scheduler.h"
#include "bundle_info.h"

namespace OHOS {
namespace AAFwk {
class AppStateCallbackMock : public AppStateCallback {
public:
    AppStateCallbackMock()
    {}
    virtual ~AppStateCallbackMock()
    {}
    MOCK_METHOD2(OnAbilityRequestDone, void(const sptr<IRemoteObject>&, const int32_t));
    MOCK_METHOD1(OnAppStateChanged, void(const AppInfo& info));
    MOCK_METHOD1(NotifyStartResidentProcess, void(std::vector<AppExecFwk::BundleInfo>&));
    MOCK_METHOD2(NotifyStartKeepAliveProcess, void(std::vector<AppExecFwk::BundleInfo>&, pid_t));
    MOCK_METHOD2(NotifyAppPreCache, void(int32_t, int32_t));
    MOCK_METHOD4(RecordAppExitSignalReason, void(int32_t, int32_t, int32_t, std::string&));
};
}  // namespace AAFwk
}  // namespace OHOS

#endif
