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

#include "dump_runtime_helper.h"

#include <dfx_signal_handler.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include "app_mgr_client.h"
#include "faultloggerd_client.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "singleton.h"
#include "dfx_jsnapi.h"
#include "parameters.h"
#include "ffrt.h"
#include "directory_ex.h"
#include "storage_acl.h"
#ifdef CJ_FRONTEND
#include "cj_runtime.h"
#endif

namespace OHOS {
namespace AppExecFwk {
const char *MODULE_NAME = "hiviewdfx.jsLeakWatcher";
const char *CHECK = "check";
const char *REQUIRE_NAPI = "requireNapi";

static bool g_betaVersion = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "beta";
static bool g_developMode = (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "enable") ||
                            (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "true");
static int g_oomDumpProcessQuota = 0;
static uint64_t g_lastOOMDumpTime = 0;
static constexpr const char* const EVENT_XATTR_NAME = "user.appevent";
static constexpr const char* const OOM_QUOTA_XATTR_NAME = "user.oomdump.quota";
static constexpr const char* const PROPERTY2C = "user.oomdumptelemetry.quota";
static constexpr const char* const HIAPPEVENT_PATH = "/data/storage/el2/base/cache/hiappevent";
static constexpr const char* const OOM_QUOTA_PATH = "/data/storage/el2/base/cache/rawheap";
static constexpr const char* const JS_HEAP_LOGTYPE = "user.event_config.js_heap_logtype";
static constexpr const char* const EVENT_RAWHEAP = "event_rawheap";
static constexpr uint64_t OOM_DUMP_INTERVAL = 7 * 24 * 60 * 60;
static constexpr uint64_t OOM_DUMP_SPACE_LIMIT = 30ull * 1024 * 1024 * 1024;
static constexpr uint32_t EVENT_RESOURCE_OVERLIMIT_MASK = 6;
static constexpr uint64_t BIT_MASK = 1;
static constexpr uint32_t BUF_SIZE_256 = 256;
static constexpr int DECIMAL_BASE = 10;
static constexpr int KB_PER_MB = 1024;

enum {
    INDEX_DELIVERY_TS = 0,
    INDEX_COMPRESSED_TS_0,
    INDEX_COMPRESSED_TS_1,
    INDEX_COMPRESSED_TS_2,
    INDEX_COMPRESSED_TS_3,
    INDEX_COMPRESSED_TS_4,
    INDEX_APP_QUOTA,
    INDEX_HAS_SENT,
    INDEX_ROM_RSV_SIZE,
    PROPERTY2C_SIZE
};

DumpRuntimeHelper::DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application)
    : application_(application)
{}

bool DumpRuntimeHelper::Check2DOOMDumpOpt()
{
    char* env = getenv("DFX_RESOURCE_OVERLIMIT_OPTIONS");
    if (env == nullptr) {
        return false;
    }
    return strstr(env, "oomdump:enable") != nullptr;
}

void DumpRuntimeHelper::SetAppFreezeFilterCallback()
{
    CreateDirDelay(OOM_QUOTA_PATH);
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return;
    }
    auto appfreezeFilterCallback =
        [this] (const int32_t pid, const bool needDecreaseQuota, std::string &eventConfig) -> bool {
        eventConfig = GetEventConfig(JS_HEAP_LOGTYPE);
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (client == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null client");
            return false;
        }
        if (!CheckOomdumpSwitch()) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to check oomdump switch");
            return false;
        }
        if (g_betaVersion || g_developMode || !needDecreaseQuota) {
            TAG_LOGI(AAFwkTag::APPKIT, "no need to check quota, just dump."
                " beta: %{public}d, develop: %{public}d, hidumper: %{public}d",
                g_betaVersion, g_developMode, !needDecreaseQuota);
            client->SetAppFreezeFilter(pid);
            return true;
        }
        bool ret2custom = (eventConfig == EVENT_RAWHEAP);
        bool ret2D = Check2DQuota(needDecreaseQuota);
        bool ret2C = Check2CQuota();
        if (!ret2custom && !ret2D && !ret2C) {
            TAG_LOGI(AAFwkTag::APPKIT, "check custom|2C|2D quota both failed, no dump.");
            return false;
        }
        TAG_LOGI(AAFwkTag::APPKIT, "check success, will dump. custom: %{public}d 2C: %{public}d, 2D: %{public}d",
                 ret2custom, ret2C, ret2D);
        client->SetAppFreezeFilter(pid);
        return true;
    };
    auto vm = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetEcmaVm();
    panda::DFXJSNApi::SetAppFreezeFilterCallback(vm, appfreezeFilterCallback);
}

bool DumpRuntimeHelper::CheckOomdumpSwitch()
{
    // if value is disable, then no dump whatever
    std::string switchValue = OHOS::system::GetParameter("hiview.oomdump.switch", "unknown");
    TAG_LOGI(AAFwkTag::APPKIT, "oomdump switch: %{public}s", switchValue.c_str());
    return switchValue != "disable";
}

bool DumpRuntimeHelper::Check2CQuota()
{
    std::vector<int64_t> quota2C;
    if (!GetQuota(OOM_QUOTA_PATH, PROPERTY2C, quota2C, PROPERTY2C_SIZE)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to GetQuota for PROPERTY2C");
        return false;
    }

    int compressQuota = GetCompressQuota(quota2C);
    int appQuota = static_cast<int>(quota2C[INDEX_APP_QUOTA]);
    int leftQuota = MIN(compressQuota, appQuota) - static_cast<int>(quota2C[INDEX_HAS_SENT]);
    if (leftQuota <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid quota, compress: %{public}d, app: %{public}d, sent: %{public}" PRId64,
            compressQuota, appQuota, quota2C[INDEX_HAS_SENT]);
        return false;
    }

    uint64_t now = GetCurrentTimestamp();
    if (quota2C[INDEX_DELIVERY_TS] < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid quota2C[%{public}d]: %{public}" PRId64,
            INDEX_DELIVERY_TS, quota2C[INDEX_DELIVERY_TS]);
        return false;
    }
    uint64_t deliveryTs = static_cast<uint64_t>(quota2C[INDEX_DELIVERY_TS]);
    if (now < deliveryTs || now - deliveryTs > OOM_DUMP_INTERVAL) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid deliveryTs: %{public}" PRIu64, deliveryTs);
        return false;
    }

    uint64_t spaceQuota = static_cast<uint64_t>(quota2C[INDEX_ROM_RSV_SIZE]) * KB_PER_MB;
    if (!CheckOOMFreeSpace(spaceQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "rom free space is less than spaceQuota: %{public}" PRIu64, spaceQuota);
        return false;
    }

    TAG_LOGE(AAFwkTag::APPKIT, "success check 2C Quota");
    return true;
}

bool DumpRuntimeHelper::Check2DQuota(bool needDecreaseQuota)
{
    if (!Check2DOOMDumpOpt()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check2DOOMDumpOpt failed");
        return false;
    }
    uint64_t time = 0ull;
    int quota = 0;
    int value = OHOS::system::GetIntParameter("persist.hiview.oomdump.process.maxcount", 0);
    if (value < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid persist.hiview.oomdump.process.maxcount: %{public}d", value);
        return false;
    }
    uint32_t oomDumpProcessMaxQuota = static_cast<uint32_t>(value);
    if (!Get2DQuota(OOM_QUOTA_PATH, OOM_QUOTA_XATTR_NAME, time, quota)) {
        TAG_LOGI(AAFwkTag::APPKIT, "failed to Get2DQuota, need to Init2DOOMDumpQuota");
        if (!Init2DOOMDumpQuota(OOM_QUOTA_PATH, oomDumpProcessMaxQuota)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed Init2DOOMDumpQuota");
            return false;
        }
    }
    value = OHOS::system::GetIntParameter("persist.hiview.oomdump.maxcount", 0);
    if (value < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid persist.hiview.oomdump.maxcount: %{public}d", value);
        return false;
    }
    uint32_t oomDumpMaxQuota = static_cast<uint32_t>(value);
    if (!Check2DOOMDumpQuota(oomDumpMaxQuota, oomDumpProcessMaxQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check2DOOMDumpQuota failed");
        return false;
    }
    Set2DQuota(OOM_QUOTA_PATH, GetCurrentTimestamp(), g_oomDumpProcessQuota - 1);

    TAG_LOGI(AAFwkTag::APPKIT, "succeed to Check2DQuota");
    return true;
}

void DumpRuntimeHelper::DumpJsHeap(const OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return;
    }
    if (runtime->GetLanguage() != OHOS::AbilityRuntime::Runtime::Language::JS) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime language is not JS");
        return;
    }
    if (info.needLeakobj) {
        std::string checkList = "";
        GetCheckList(runtime, checkList);
        WriteCheckList(checkList);
    }

    if (info.needSnapshot == true) {
        runtime->DumpHeapSnapshot(info.tid, info.needGc, info.needBinary, info.needClearNodeIdCache);
    } else {
        if (info.needGc == true) {
            runtime->ForceFullGC(info.tid);
        }
    }
}

void DumpRuntimeHelper::DumpCjHeap(const OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
#ifdef CJ_FRONTEND
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjEnv");
        return;
    }
    if (info.needSnapshot == true) {
        int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::CJ_HEAP_SNAPSHOT));
        if (fd < 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "fd:%{public}d.\n", fd);
            return;
        }
        cjEnv->dumpHeapSnapshot(fd);
        close(fd);
        return;
    }
    if (info.needGc == true) {
        cjEnv->forceFullGC();
    }
#endif
}

void DumpRuntimeHelper::GetCheckList(const std::unique_ptr<AbilityRuntime::Runtime> &runtime, std::string &checkList)
{
    if (runtime->GetLanguage() != AbilityRuntime::Runtime::Language::JS) {
        TAG_LOGE(AAFwkTag::APPKIT, "current language not js");
        return;
    }
    AbilityRuntime::JsRuntime &jsruntime = static_cast<AbilityRuntime::JsRuntime&>(*runtime);
    AbilityRuntime::HandleScope handleScope(jsruntime);
    auto env = jsruntime.GetNapiEnv();

    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value requireValue = GetJsLeakModule(env, global);
    if (requireValue == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null requireValue");
        return;
    }
    napi_value result = GetMethodCheck(env, requireValue, global);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null result");
        return;
    }

    size_t checkListSize = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &checkListSize);
    checkList.resize(checkListSize + 1);
    napi_get_value_string_utf8(env, result, &checkList[0], checkListSize + 1, &checkListSize);
}

napi_value DumpRuntimeHelper::GetJsLeakModule(napi_env env, napi_value global)
{
    napi_value napiFunc = nullptr;
    napi_status status = napi_get_named_property(env, global, REQUIRE_NAPI, &napiFunc);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    napi_value moduleName = nullptr;
    status = napi_create_string_utf8(env, MODULE_NAME, strlen(MODULE_NAME), &moduleName);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    napi_value param[1] = {moduleName};
    napi_value requireValue = nullptr;
    status = napi_call_function(env, global, napiFunc, 1, &param[0], &requireValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    return requireValue;
}

napi_value DumpRuntimeHelper::GetMethodCheck(napi_env env, napi_value requireValue, napi_value global)
{
    napi_value methodCheck = nullptr;
    napi_status status = napi_get_named_property(env, requireValue, CHECK, &methodCheck);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    napi_valuetype valuetype = napi_undefined;
    status = napi_typeof(env, methodCheck, &valuetype);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, %{public}d", status);
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_call_function(env, global, methodCheck, 0, nullptr, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    return result;
}

void DumpRuntimeHelper::WriteCheckList(const std::string &checkList)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::JS_HEAP_LEAK_LIST));
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "fd:%{public}d.\n", fd);
        return;
    }
    if (write(fd, checkList.c_str(), strlen(checkList.c_str())) == -1) {
        TAG_LOGE(AAFwkTag::APPKIT, "fd:%{public}d, errno:%{public}d.\n", fd, errno);
        close(fd);
        return;
    }
    close(fd);
}

void DumpRuntimeHelper::CreateDirDelay(const std::string &path)
{
    ffrt::submit([=] {
        if (!CreateDir(path)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to create %{public}s", path.c_str());
            return;
        }
        TAG_LOGI(AAFwkTag::APPKIT, "success to create %{public}s", path.c_str());
        }, {}, {}, {ffrt::task_attr().name("ffrt_dfr_CreateDir")});
}

bool DumpRuntimeHelper::Init2DOOMDumpQuota(const std::string &path, uint32_t oomDumpProcessMaxQuota)
{
    uint64_t currentTime = GetCurrentTimestamp();
    if (!Set2DQuota(path, currentTime, oomDumpProcessMaxQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Set2DQuota failed, current: %{public}" PRIu64 ", quota: %{public}d",
            currentTime, oomDumpProcessMaxQuota);
        return false;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Init dumpTime: %{public}" PRIu64 ", quota: %{public}d.",
        currentTime, oomDumpProcessMaxQuota);
    return true;
}

bool DumpRuntimeHelper::CreateDir(const std::string &path)
{
    if (IsFileExists(path)) {
        TAG_LOGI(AAFwkTag::APPKIT, "File existed. dir: %{public}s", path.c_str());
        return true;
    }
    if (mkdir(path.c_str(), S_IRWXU) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create dir: %{public}s", path.c_str());
        return false;
    }
    constexpr mode_t defaultLogDirMode = 0770;
    if (!OHOS::ChangeModeDirectory(path, defaultLogDirMode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to changeMode %{public}s", path.c_str());
        rmdir(path.c_str());
        return false;
    }
    if (OHOS::StorageDaemon::AclSetAccess(path, "g:1201:rwx") != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to AclSetAccess, path: %{public}s", path.c_str());
        rmdir(path.c_str());
        return false;
    }
    return true;
}

bool DumpRuntimeHelper::Check2DOOMDumpQuota(uint32_t oomDumpMaxQuota, uint32_t oomDumpProcessMaxQuota)
{
    if (!CheckAppListenedEvents(HIAPPEVENT_PATH)) {
        TAG_LOGI(AAFwkTag::APPKIT, "Not subscribe oom event.");
        return false;
    }
    if (oomDumpMaxQuota <= 0) {
        TAG_LOGI(AAFwkTag::APPKIT, "The whole machine quota has been exhausted, quota=%{public}u", oomDumpMaxQuota);
        return false;
    }
    if (!CheckOOMFreeSpace(OOM_DUMP_SPACE_LIMIT)) {
        TAG_LOGI(AAFwkTag::APPKIT, "Device space is not enough.");
        return false;
    }
    if (!Get2DQuota(OOM_QUOTA_PATH, OOM_QUOTA_XATTR_NAME, g_lastOOMDumpTime, g_oomDumpProcessQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check2DOOMDumpQuota failed to get quota.");
        return false;
    }
    uint64_t currentTime = GetCurrentTimestamp();
    if (currentTime - g_lastOOMDumpTime <= OOM_DUMP_INTERVAL) {
        if (g_oomDumpProcessQuota <= 0) {
            TAG_LOGI(AAFwkTag::APPKIT, "Weekly quota has been exhausted, quota=%{public}d", g_oomDumpProcessQuota);
            return false;
        }
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "Over one week, reset process quota=%{public}u", oomDumpProcessMaxQuota);
        g_oomDumpProcessQuota = static_cast<int>(oomDumpProcessMaxQuota);
        if (!Set2DQuota(OOM_QUOTA_PATH, currentTime, oomDumpProcessMaxQuota)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to reset quota.");
            return false;
        }
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Last oom time=%{public}" PRIu64 ", currentTime=%{public}" PRIu64 ", quota=%{public}d",
        g_lastOOMDumpTime, currentTime, g_oomDumpProcessQuota);
    return true;
}

bool DumpRuntimeHelper::GetQuota(const std::string &path, const std::string &property,
                                 std::vector<int64_t> &output, size_t size)
{
    std::string value;
    if (!GetDirXattr(path, property, value)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get xattr. preperty: %{public}s", property.c_str());
        return false;
    }
    size_t commaCount = static_cast<size_t>(std::count(value.begin(), value.end(), ','));
    if (commaCount != size - 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid number of commas: %{public}zu", commaCount);
        return false;
    }

    std::vector<int64_t> rtn;
    std::stringstream ss(value);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        if (segment.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid segment: empty value found, index: %{public}zu", rtn.size());
            return false;
        }
        long long tmp;
        if (!SafeStoll(segment, tmp)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to SafeStoll, segment: %{public}s", segment.c_str());
            return false;
        }
        rtn.push_back(static_cast<int64_t>(tmp));
    }

    if (rtn.size() != size) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid number of entries: %{public}zu", rtn.size());
        return false;
    }

    output = rtn;
    return true;
}

bool DumpRuntimeHelper::Get2DQuota(const std::string &path, const std::string &property,
                                   uint64_t &timestamp, int &quota)
{
    std::string value;
    if (!GetDirXattr(path, property, value)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get xattr. path: %{public}s, preperty: %{public}s",
            path.c_str(), property.c_str());
        return false;
    }
    std::istringstream ss(value);
    char delimiter;
    if (!(ss >> timestamp >> delimiter >> quota) || delimiter != ',') {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to decode value: %{public}s", value.c_str());
        return false;
    }
    return true;
}

bool DumpRuntimeHelper::Set2DQuota(const std::string &path, uint64_t timestamp, int newQuota)
{
    std::ostringstream value;
    value << timestamp << "," << newQuota;
    if (newQuota < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid quota.");
        return false;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Set quota=%{public}d", newQuota);
    return SetDirXattr(path, OOM_QUOTA_XATTR_NAME, value.str());
}

uint64_t DumpRuntimeHelper::GetCurrentTimestamp()
{
    return static_cast<uint64_t>(time(nullptr));
}

bool DumpRuntimeHelper::CheckOOMFreeSpace(uint64_t maxSpace)
{
    struct statvfs st;
    if (statvfs("/data/storage/el2/base/", &st) != 0) {
        return false;
    }

    unsigned long freeSize = st.f_bsize * st.f_bfree;
    if (freeSize <= maxSpace) {
        return false;
    }
    return true;
}

uint64_t DumpRuntimeHelper::GetMaskFromDirXattr(const std::string &path)
{
    std::string value;
    if (!GetDirXattr(path, EVENT_XATTR_NAME, value)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get subscirbe xattr.");
        return 0;
    }
    return static_cast<uint64_t>(std::strtoull(value.c_str(), nullptr, 0));
}

bool DumpRuntimeHelper::CheckAppListenedEvents(const std::string &path)
{
    uint64_t eventsMask = GetMaskFromDirXattr(path);
    if (eventsMask == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get events mask for path:%{public}s", path.c_str());
        return false;
    }
    if (!(eventsMask & (BIT_MASK << EVENT_RESOURCE_OVERLIMIT_MASK))) {
        TAG_LOGI(AAFwkTag::APPKIT, "Unlistened event for path:%{public}s", path.c_str());
        return false;
    }
    return true;
}

bool DumpRuntimeHelper::IsFileExists(const std::string &file)
{
    return access(file.c_str(), F_OK) == 0;
}

bool DumpRuntimeHelper::SetDirXattr(const std::string &path, const std::string &name, const std::string &value)
{
    return setxattr(path.c_str(), name.c_str(), value.c_str(), strlen(value.c_str()), 0) == 0;
}

bool DumpRuntimeHelper::GetDirXattr(const std::string &path, const std::string &name, std::string &value)
{
    char buf[BUF_SIZE_256] = {0};
    if (getxattr(path.c_str(), name.c_str(), buf, sizeof(buf) - 1) == -1) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed getxattr, name: %{public}s, err: %{public}d:%{public}s",
                 name.c_str(), errno, strerror(errno));
        return false;
    }
    value = buf;
    return true;
}

bool DumpRuntimeHelper::SafeStoll(const std::string &str, long long &value)
{
    value = 0;
    size_t start = 0;
    bool isNegative = false;

    if (str.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "str is empty");
        return false;
    }

    if (str[0] == '-') {
        isNegative = true;
        start = 1;
    } else if (str[0] == '+') {
        start = 1;
    }

    for (size_t i = start; i < str.size(); ++i) {
        if (!isdigit(str[i])) {
            TAG_LOGE(AAFwkTag::APPKIT, "digit check failed. str: %{public}s, index: %{public}zu", str.c_str(), i);
            return false;
        }
        if (value > (LLONG_MAX - (str[i] - '0')) / DECIMAL_BASE) {
            TAG_LOGE(AAFwkTag::APPKIT, "out of range, str: %{public}s", str.c_str());
            return false;
        }
        value = value * DECIMAL_BASE + (str[i] - '0');
    }

    if (isNegative) {
        value = -value;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "success, str: %{public}s, result: %{public}lld", str.c_str(), value);
    return true;
}

int DumpRuntimeHelper::GetCompressQuota(const std::vector<int64_t> &quotas)
{
    int ret = 0;
    uint64_t now = static_cast<uint64_t>(time(nullptr));
    for (int i = INDEX_COMPRESSED_TS_0; i <= INDEX_COMPRESSED_TS_4; i++) {
        uint64_t compressTs = static_cast<uint64_t>(quotas[i]);
        if (now > compressTs && now - compressTs > OOM_DUMP_INTERVAL) {
            ret++;
        }
    }

    return ret;
}

bool DumpRuntimeHelper::SplitPropertyByComma(const std::string &property, std::string &runningId, std::string &value)
{
    size_t commaPos = property.find(',');
    if (commaPos == std::string::npos) {
        TAG_LOGE(AAFwkTag::APPKIT, "no comma in property");
        return false;
    }

    runningId = property.substr(0, commaPos);
    value = property.substr(commaPos + 1);
    return true;
}

std::string DumpRuntimeHelper::GetEventConfig(const std::string &key)
{
    std::string property;
    if (!GetDirXattr(OOM_QUOTA_PATH, key, property)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to GetDirXattr, key: %{public}s", key.c_str());
        return "";
    }
    std::string customRunningId;
    std::string value;
    if (!SplitPropertyByComma(property, customRunningId, value)) {
        TAG_LOGW(AAFwkTag::APPKIT, "failed to SplitPropertyByComma, property: %{public}s", property.c_str());
        return "";
    }
    std::string curRunningId = DFX_GetAppRunningUniqueId();
    if (curRunningId.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "curRunningId is empty");
        return "";
    }
    if (customRunningId != curRunningId) {
        TAG_LOGW(AAFwkTag::APPKIT, "runningId not match, customRunningId: %{public}s, curRunningId: %{public}s",
            customRunningId.c_str(), curRunningId.c_str());
        return "";
    }

    TAG_LOGI(AAFwkTag::APPKIT, "succeed to getEventConfig, value: %{public}s", value.c_str());
    return value;
}
} // namespace AppExecFwk
} // namespace OHOS
