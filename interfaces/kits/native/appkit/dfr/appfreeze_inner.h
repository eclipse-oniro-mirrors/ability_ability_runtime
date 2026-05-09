/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_ABILITY_APPFREEZE_LOG_CLIENT_H
#define OHOS_ABILITY_ABILITY_APPFREEZE_LOG_CLIENT_H
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <set>
#include <sys/types.h>
#include <condition_variable>

#include "refbase.h"
#include "singleton.h"

#include "event_handler.h"
#include "app_mgr_client.h"
#include "app_mgr_interface.h"
#include "application_impl.h"
#include "fault_data.h"
#include "task_handler_wrap.h"

#define OHOS_TEMP_FAILURE_RETRY(exp)               \
    ({                                             \
        long int _rc;                              \
        do {                                       \
            _rc = (long int)(exp);                 \
        } while ((_rc == -1) && (errno == EINTR)); \
        _rc;                                       \
    })

namespace OHOS {
namespace AppExecFwk {
class AppfreezeInner {
public:
    AppfreezeInner();
    ~AppfreezeInner();
    static std::shared_ptr<AppfreezeInner> GetInstance();
    static void DestroyInstance();
    static void SetMainHandler(const std::shared_ptr<EventHandler>& eventHandler);
    void SetApplicationInfo(const std::shared_ptr<ApplicationInfo>& applicationInfo);
    void ThreadBlock(std::atomic_bool& isSixSecondEvent, uint64_t schedTime = 0,
        uint64_t now = 0, bool isInBackground = false);
    void ChangeFaultDateInfo(FaultData& faultData, const std::string& msgContent);
    void AppfreezeHandleOverReportCount(bool isSixSecondEvent);
    void GetMainHandlerDump(std::string& msgContent);
    int AppfreezeHandle(const FaultData& faultInfo, bool onlyMainThread);
    int AcquireStack(const FaultData& faultInfo, bool onlyMainThread);
    void SetAppDebug(bool isAppDebug);
    void SetAppInForeground(bool isInForeground);
    void SetMainThreadSample(bool isEnableMainThreadSample);
    void SetReportLifeCycleAsAppfreeze(bool reportLifecycleToFreeze);
    void SetAppfreezeApplication(const std::shared_ptr<OHOSApplication> &application);
    std::string GetProcessLifeCycle();

private:
    static std::weak_ptr<EventHandler> appMainHandler_;
    std::weak_ptr<ApplicationInfo> applicationInfo_;
    void AppFreezeRecovery();
    int NotifyANR(const FaultData& faultData);
    bool IsExitApp(const std::string& name);
    bool IsAppFreeze(const std::string& name);
    bool IsAppFreezeWarning(const std::string& name);
    bool IsHandleAppfreeze();
    std::string GetProcStatm(int32_t pid);
    bool GetAppInForeground();
    bool GetMainThreadSample();
    bool GetReportLifeCycleAsAppfreeze();
    void EnableFreezeSample(FaultData& newFaultData);
    void ReportAppfreezeTask(const FaultData& faultData, bool onlyMainThread);
    std::string LogFormat(size_t totalSize, size_t objectSize);
    void GetApplicationInfo(FaultData& faultData);
    bool GetProcessStartTime(pid_t tid, unsigned long long &startTime);
    bool ReadFdToString(int fd, std::string& content);
    int TransformHicollieFaultNumber(const std::string& faultName);
    std::string GetMainStackDump(int32_t pid);

    static std::mutex singletonMutex_;
    static std::shared_ptr<AppfreezeInner> instance_;
    bool isAppDebug_ = false;
    bool isInForeground_ = true;
    bool isEnableMainThreadSample_ = false;
    bool reportLifecycleToFreeze_ = false;
    std::mutex handlingMutex_;
    std::list<FaultData> handlinglist_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> appfreezeInnerTaskHandler_;
    std::shared_ptr<OHOSApplication> application_ = nullptr;

    std::mutex mainStackMutex_;
    std::condition_variable mainStackCv_;
    std::string lastMainStack_ = "";
    std::atomic<int64_t> lastMainStackTime_ = 0;
};

class MainHandlerDumper : public Dumper {
public:
    virtual void Dump(const std::string &message) override;
    virtual std::string GetTag() override;
    std::string GetDumpInfo();
private:
    std::string dumpInfo;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_ABILITY_APPFREEZE_LOG_CLIENT_H