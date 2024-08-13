/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ohos_js_environment_impl.h"

#include "commonlibrary/ets_utils/js_sys_module/console/console.h"
#include "commonlibrary/ets_utils/js_sys_module/timer/timer.h"
#include "hilog_tag_wrapper.h"
#include "js_utils.h"
#include "js_worker.h"
#include "ohos_loop_handler.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    std::shared_ptr<AppExecFwk::EventHandler> g_eventHandler = nullptr;
}
void OHOSJsEnvironmentImpl::PostTaskToHandler(void* handler, uv_io_cb func, void* work, int status, int priority)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (!func || !work) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid parameters");
        return;
    }

    auto task = [func, work, status]() {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Do uv work");
        func(work, status);
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Do uv work end");
    };

    AppExecFwk::EventQueue::Priority prio = AppExecFwk::EventQueue::Priority::IMMEDIATE;
    switch (priority) {
        case uv_qos_t::uv_qos_user_initiated:
            prio = AppExecFwk::EventQueue::Priority::IMMEDIATE;
            break;
        case uv_qos_t::uv_qos_utility:
            prio = AppExecFwk::EventQueue::Priority::LOW;
            break;
        case uv_qos_t::uv_qos_background:
            prio = AppExecFwk::EventQueue::Priority::IDLE;
            break;
        default:
            prio = AppExecFwk::EventQueue::Priority::HIGH;
            break;
    }

    if (g_eventHandler  == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid parameters");
        return;
    }
    g_eventHandler->PostTask(task, prio);
}
OHOSJsEnvironmentImpl::OHOSJsEnvironmentImpl()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
}

OHOSJsEnvironmentImpl::OHOSJsEnvironmentImpl(const std::shared_ptr<AppExecFwk::EventRunner>& eventRunner)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (eventRunner != nullptr) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Create event handler");
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
        if (eventRunner.get() == AppExecFwk::EventRunner::GetMainEventRunner().get()) {
            g_eventHandler = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
        }
    }
}

OHOSJsEnvironmentImpl::~OHOSJsEnvironmentImpl()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
}

void OHOSJsEnvironmentImpl::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(task, name, delayTime);
    }
}

void OHOSJsEnvironmentImpl::PostSyncTask(const std::function<void()>& task, const std::string& name)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (eventHandler_ != nullptr) {
        eventHandler_->PostSyncTask(task, name);
    }
}

void OHOSJsEnvironmentImpl::RemoveTask(const std::string& name)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(name);
    }
}

void OHOSJsEnvironmentImpl::InitTimerModule(NativeEngine* engine)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    CHECK_POINTER(engine);
    auto ret = JsSysModule::Timer::RegisterTime(reinterpret_cast<napi_env>(engine));
    if (!ret) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Register timer failed");
    }
}

void OHOSJsEnvironmentImpl::InitConsoleModule(NativeEngine* engine)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    JsSysModule::Console::InitConsoleModule(reinterpret_cast<napi_env>(engine));
}

bool OHOSJsEnvironmentImpl::InitLoop(NativeEngine* engine, bool isStage)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    CHECK_POINTER_AND_RETURN(engine, false);
    auto uvLoop = engine->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to get backend fd from uv loop");
        return false;
    }
    uv_run(uvLoop, UV_RUN_NOWAIT);

    if (eventHandler_ != nullptr) {
        uint32_t events = AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
        eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<OHOSLoopHandler>(uvLoop), "uvLoopTask");
        TAG_LOGD(AAFwkTag::JSRUNTIME, "uv_register_task_to_event, isStage: %{public}d", isStage);
        if (isStage && (eventHandler_->GetEventRunner()).get() == AppExecFwk::EventRunner::GetMainEventRunner().get()) {
            uv_register_task_to_event(uvLoop, PostTaskToHandler, nullptr);
        }
    }

    return true;
}

void OHOSJsEnvironmentImpl::DeInitLoop(NativeEngine* engine)
{
    CHECK_POINTER(engine);
    auto uvLoop = engine->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd >= 0 && eventHandler_ != nullptr) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    uv_unregister_task_to_event(uvLoop);
    RemoveTask(TIMER_TASK);
}

void OHOSJsEnvironmentImpl::InitWorkerModule(NativeEngine* engine, std::shared_ptr<JsEnv::WorkerInfo> workerInfo)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    CHECK_POINTER(engine);
    CHECK_POINTER(workerInfo);
    engine->SetInitWorkerFunc(InitWorkerFunc);
    engine->SetOffWorkerFunc(OffWorkerFunc);
    engine->SetGetAssetFunc(AssetHelper(workerInfo));
    engine->SetApiVersion(workerInfo->apiTargetVersion);

    engine->SetGetContainerScopeIdFunc(GetContainerId);
    engine->SetInitContainerScopeFunc(UpdateContainerScope);
    engine->SetFinishContainerScopeFunc(RestoreContainerScope);
}

void OHOSJsEnvironmentImpl::InitSyscapModule()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
}
} // namespace AbilityRuntime
} // namespace OHOS
