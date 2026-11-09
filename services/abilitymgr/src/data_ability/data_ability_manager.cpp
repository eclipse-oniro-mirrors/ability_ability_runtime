/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "data_ability_manager.h"

#include "ability_manager_service.h"
#include "ability_resident_process_rdb.h"
#include "ability_util.h"
#include "connection_state_manager.h"

namespace OHOS {
namespace AAFwk {
using namespace std::chrono;
using namespace std::placeholders;

namespace {
constexpr bool DEBUG_ENABLED = false;
constexpr system_clock::duration DATA_ABILITY_LOAD_TIMEOUT = 11000ms;
constexpr int32_t USER_ID_NO_HEAD = 0;
}  // namespace

DataAbilityManager::DataAbilityManager()
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");
}

DataAbilityManager::~DataAbilityManager()
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");
}

sptr<IAbilityScheduler> DataAbilityManager::Acquire(
    const AbilityRequest &abilityRequest, bool tryBind, const sptr<IRemoteObject> &client, bool isNotHap)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not dataability");
        return nullptr;
    }

    if (abilityRequest.abilityInfo.bundleName.empty() || abilityRequest.abilityInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid name");
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> clientAbilityRecord;
    const std::string dataAbilityName(abilityRequest.abilityInfo.bundleName + '.' + abilityRequest.abilityInfo.name);

    if (client && !isNotHap) {
        clientAbilityRecord = Token::GetAbilityRecordByToken(client);
        if (!clientAbilityRecord) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid client token");
            return nullptr;
        }
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "ability '%{public}s' acquiring data ability '%{public}s'...",
            clientAbilityRecord->GetAbilityInfo().name.c_str(), dataAbilityName.c_str());
    } else {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "loading dataability '%{public}s'...", dataAbilityName.c_str());
    }

    std::lock_guard<ffrt::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    DataAbilityRecordPtr dataAbilityRecord;

    auto it = dataAbilityRecordsLoaded_.find(dataAbilityName);
    if (it == dataAbilityRecordsLoaded_.end()) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "data ability not existed, loading...");
        dataAbilityRecord = LoadLocked(dataAbilityName, abilityRequest);
    } else {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "data ability existed");
        dataAbilityRecord = it->second;
    }

    if (!dataAbilityRecord) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "'%{public}s' failed", dataAbilityName.c_str());
        return nullptr;
    }

    auto scheduler = dataAbilityRecord->GetScheduler();
    if (!scheduler) {
        if (DEBUG_ENABLED) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "'%{public}s' removing",
                dataAbilityName.c_str());
        }
        auto it = dataAbilityRecordsLoaded_.find(dataAbilityName);
        if (it != dataAbilityRecordsLoaded_.end()) {
            dataAbilityRecordsLoaded_.erase(it);
        }
        return nullptr;
    }

    if (client) {
        dataAbilityRecord->AddClient(client, tryBind, isNotHap);
    }

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    ReportDataAbilityAcquired(client, isNotHap, dataAbilityRecord);

    return scheduler;
}

int DataAbilityManager::Release(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &client, bool isNotHap)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    CHECK_POINTER_AND_RETURN(scheduler, ERR_NULL_OBJECT);
    CHECK_POINTER_AND_RETURN(client, ERR_NULL_OBJECT);

    std::lock_guard<ffrt::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    DataAbilityRecordPtr dataAbilityRecord;

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (it->second && it->second->GetScheduler() &&
            it->second->GetScheduler()->AsObject() == scheduler->AsObject()) {
            dataAbilityRecord = it->second;
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "Releasing '%{public}s'...", it->first.c_str());
            break;
        }
    }

    if (!dataAbilityRecord) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability");
        return ERR_UNKNOWN_OBJECT;
    }

    auto abilityRecord = dataAbilityRecord->GetAbilityRecord();
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_UNKNOWN_OBJECT);
    auto abilityMs = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(abilityMs, GET_ABILITY_SERVICE_FAILED);
    int result = abilityMs->JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "JudgeAbilityVisibleControl error");
        return result;
    }

    if (dataAbilityRecord->GetClientCount(client) == 0) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "client wrong");
        return ERR_UNKNOWN_OBJECT;
    }

    dataAbilityRecord->RemoveClient(client, isNotHap);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    ReportDataAbilityReleased(client, isNotHap, dataAbilityRecord);

    return ERR_OK;
}

bool DataAbilityManager::ContainsDataAbility(const sptr<IAbilityScheduler> &scheduler)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    CHECK_POINTER_AND_RETURN(scheduler, ERR_NULL_OBJECT);

    std::lock_guard<ffrt::mutex> locker(mutex_);
    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (it->second && it->second->GetScheduler() &&
            it->second->GetScheduler()->AsObject() == scheduler->AsObject()) {
            return true;
        }
    }

    return false;
}

int DataAbilityManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    CHECK_POINTER_AND_RETURN(scheduler, ERR_NULL_OBJECT);
    CHECK_POINTER_AND_RETURN(token, ERR_NULL_OBJECT);

    std::lock_guard<ffrt::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "attaching dataability");

    auto record = Token::GetAbilityRecordByToken(token);
    std::string abilityName = "";
    if (record != nullptr) {
        abilityName = record->GetAbilityInfo().name;
    }

    DataAbilityRecordPtr dataAbilityRecord;
    auto it = dataAbilityRecordsLoading_.begin();
    for (; it != dataAbilityRecordsLoading_.end(); ++it) {
        if (it->second && it->second->GetToken() == token) {
            dataAbilityRecord = it->second;
            break;
        }
    }

    if (!dataAbilityRecord) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "attaching '%{public}s' not loaded",
            abilityName.c_str());
        return ERR_UNKNOWN_OBJECT;
    }

    return dataAbilityRecord->Attach(scheduler);
}

int DataAbilityManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    CHECK_POINTER_AND_RETURN(token, ERR_NULL_OBJECT);

    std::lock_guard<ffrt::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "transition done %{public}d", state);

    DataAbilityRecordPtrMap::iterator it;
    DataAbilityRecordPtr dataAbilityRecord;
    auto record = Token::GetAbilityRecordByToken(token);
    std::string abilityName = "";
    if (record != nullptr) {
        abilityName = record->GetAbilityInfo().name;
        record->RemoveSignatureInfo();
    }
    for (it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        if (it->second && it->second->GetToken() == token) {
            dataAbilityRecord = it->second;
            break;
        }
    }
    if (!dataAbilityRecord) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "'%{public}s' null", abilityName.c_str());
        return ERR_UNKNOWN_OBJECT;
    }

    int ret = dataAbilityRecord->OnTransitionDone(state);
    if (ret == ERR_OK) {
        dataAbilityRecordsLoaded_[it->first] = dataAbilityRecord;
        dataAbilityRecordsLoading_.erase(it);
    }

    return ret;
}

void DataAbilityManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    /* Do nothing now. */
}

void DataAbilityManager::OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "call");
    CHECK_POINTER(abilityRecord);

    {
        std::lock_guard<ffrt::mutex> locker(mutex_);
        if (DEBUG_ENABLED) {
            DumpLocked(__func__, __LINE__);
        }
        if (abilityRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::DATA) {
            // If 'abilityRecord' is a data ability server, trying to remove it from 'dataAbilityRecords_'.
            for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end();) {
                if (it->second && it->second->GetAbilityRecord() == abilityRecord) {
                    DelayedSingleton<ConnectionStateManager>::GetInstance()->HandleDataAbilityDied(it->second);
                    it->second->KillBoundClientProcesses();
                    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Removing died data ability record...");
                    it = dataAbilityRecordsLoaded_.erase(it);
                    break;
                } else {
                    ++it;
                }
            }
        }
        if (DEBUG_ENABLED) {
            DumpLocked(__func__, __LINE__);
        }
        // If 'abilityRecord' is a data ability client, tring to remove it from all servers.
        for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
            if (it->second) {
                it->second->RemoveClients(abilityRecord);
            }
        }
        if (DEBUG_ENABLED) {
            DumpLocked(__func__, __LINE__);
        }
    }

    RestartDataAbility(abilityRecord);
}

void DataAbilityManager::OnAppStateChanged(const AppInfo &info)
{
    std::lock_guard<ffrt::mutex> locker(mutex_);

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord && info.bundleName == abilityRecord->GetApplicationInfo().bundleName &&
            info.appIndex == abilityRecord->GetAppIndex() && info.instanceKey == abilityRecord->GetInstanceKey()) {
            auto appName = abilityRecord->GetApplicationInfo().name;
            auto uid = abilityRecord->GetAbilityInfo().applicationInfo.uid;
            auto isExist = [&appName, &uid](
                               const AppData &appData) { return appData.appName == appName && appData.uid == uid; };
            auto iter = std::find_if(info.appData.begin(), info.appData.end(), isExist);
            if (iter != info.appData.end()) {
                abilityRecord->SetAppState(info.state);
            }
        }
    }

    for (auto it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord && info.bundleName == abilityRecord->GetApplicationInfo().bundleName &&
            info.appIndex == abilityRecord->GetAppIndex() && info.instanceKey == abilityRecord->GetInstanceKey()) {
            auto appName = abilityRecord->GetApplicationInfo().name;
            auto uid = abilityRecord->GetAbilityInfo().applicationInfo.uid;
            auto isExist = [&appName, &uid](
                               const AppData &appData) { return appData.appName == appName && appData.uid == uid; };
            auto iter = std::find_if(info.appData.begin(), info.appData.end(), isExist);
            if (iter != info.appData.end()) {
                abilityRecord->SetAppState(info.state);
            }
        }
    }
}

std::shared_ptr<AbilityRecord> DataAbilityManager::GetAbilityRecordById(int64_t id)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    std::lock_guard<ffrt::mutex> locker(mutex_);

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord && abilityRecord->GetRecordId() == id) {
            return abilityRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityRecord> DataAbilityManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    CHECK_POINTER_AND_RETURN(token, nullptr);

    std::lock_guard<ffrt::mutex> locker(mutex_);
    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord == Token::GetAbilityRecordByToken(token)) {
            return abilityRecord;
        }
    }
    for (auto it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord == Token::GetAbilityRecordByToken(token)) {
            return abilityRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> DataAbilityManager::GetAbilityRecordByScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    CHECK_POINTER_AND_RETURN(scheduler, nullptr);

    std::lock_guard<ffrt::mutex> locker(mutex_);

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (it->second && it->second->GetScheduler() &&
            it->second->GetScheduler()->AsObject() == scheduler->AsObject()) {
            return it->second->GetAbilityRecord();
        }
    }

    return nullptr;
}

void DataAbilityManager::Dump(const char *func, int line)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Call");

    std::lock_guard<ffrt::mutex> locker(mutex_);

    DumpLocked(func, line);
}

DataAbilityManager::DataAbilityRecordPtr DataAbilityManager::LoadLocked(
    const std::string &name, const AbilityRequest &req)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "name '%{public}s'", name.c_str());

    DataAbilityRecordPtr dataAbilityRecord;

    auto it = dataAbilityRecordsLoading_.find(name);
    if (it == dataAbilityRecordsLoading_.end()) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "load failed");

        dataAbilityRecord = std::make_shared<DataAbilityRecord>(req);
        // Start data ability loading process asynchronously.
        int startResult = dataAbilityRecord->StartLoading();
        if (startResult != ERR_OK) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "dataability '%{public}d' load failed", startResult);
            return nullptr;
        }

        auto insertResult = dataAbilityRecordsLoading_.insert({name, dataAbilityRecord});
        if (!insertResult.second) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, " insert failed");
            return nullptr;
        }
    } else {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "dataability loading");
        dataAbilityRecord = it->second;
    }

    if (!dataAbilityRecord) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "'%{public}s' load failed", name.c_str());
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "wait to load");

    // Waiting for data ability loaded.
    int ret = dataAbilityRecord->WaitForLoaded(mutex_, DATA_ABILITY_LOAD_TIMEOUT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "wait failed %{public}d", ret);
        it = dataAbilityRecordsLoading_.find(name);
        if (it != dataAbilityRecordsLoading_.end()) {
            dataAbilityRecordsLoading_.erase(it);
        }
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(dataAbilityRecord->GetToken());
        return nullptr;
    }

    return dataAbilityRecord;
}

void DataAbilityManager::DumpLocked(const char *func, int line)
{
    if (func && line >= 0) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "dump at %{public}s(%{public}d)", func, line);
    } else {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "dump");
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "available count: %{public}zu", dataAbilityRecordsLoaded_.size());

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "'%{public}s':", it->first.c_str());
        if (it->second) {
            it->second->Dump();
        }
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "loading count: %{public}zu", dataAbilityRecordsLoading_.size());

    for (auto it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "'%{public}s':", it->first.c_str());
        if (it->second) {
            it->second->Dump();
        }
    }
}

void DataAbilityManager::DumpState(std::vector<std::string> &info, const std::string &args) const
{
    DataAbilityRecordPtrMap dataAbilityRecordMap;
    {
        std::lock_guard<ffrt::mutex> locker(mutex_);
        dataAbilityRecordMap = dataAbilityRecordsLoaded_;
    }
    if (!args.empty()) {
        auto it = std::find_if(dataAbilityRecordMap.begin(), dataAbilityRecordMap.end(),
            [&args](const auto &dataAbilityRecord) { return dataAbilityRecord.first.compare(args) == 0; });
        if (it != dataAbilityRecordMap.end()) {
            info.emplace_back("AbilityName [ " + it->first + " ]");
            if (it->second) {
                it->second->Dump(info);
            }
        } else {
            info.emplace_back(args + ": Nothing to dump.");
        }
    } else {
        info.emplace_back("dataAbilityRecords:");
        for (auto &&dataAbilityRecord : dataAbilityRecordMap) {
            info.emplace_back("  uri [" + dataAbilityRecord.first + "]");
            if (dataAbilityRecord.second) {
                dataAbilityRecord.second->Dump(info);
            }
        }
    }
}

void DataAbilityManager::DumpClientInfo(std::vector<std::string> &info, bool isClient,
    std::shared_ptr<DataAbilityRecord> record) const
{
    if (record == nullptr) {
        return;
    }
    record->Dump(info);
    // add dump client info
    if (isClient && record->GetScheduler() && record->GetAbilityRecord() && record->GetAbilityRecord()->IsReady()) {
        std::vector<std::string> params;
        record->GetScheduler()->DumpAbilityInfo(params, info);
        AppExecFwk::Configuration config;
        if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
            info.emplace_back("          configuration: " + config.GetName());
        }
        return;
    }
}

void DataAbilityManager::DumpSysState(std::vector<std::string> &info, bool isClient, const std::string &args) const
{
    DataAbilityRecordPtrMap dataAbilityRecordMap;
    {
        std::lock_guard<ffrt::mutex> locker(mutex_);
        dataAbilityRecordMap = dataAbilityRecordsLoaded_;
    }
    if (args.empty()) {
        info.emplace_back("  dataAbilityRecords:");
        for (auto &&dataAbilityRecord : dataAbilityRecordMap) {
            info.emplace_back("    uri [" + dataAbilityRecord.first + "]");
            DumpClientInfo(info, isClient, dataAbilityRecord.second);
        }
        return;
    }
    auto compareFunction = [&args](const auto &dataAbilityRecord) {
        return dataAbilityRecord.first.compare(args) == 0;
    };
    auto it = std::find_if(dataAbilityRecordMap.begin(), dataAbilityRecordMap.end(), compareFunction);
    if (it == dataAbilityRecordMap.end()) {
        info.emplace_back(args + ": Nothing to dump.");
        return;
    }
    info.emplace_back("AbilityName [ " + it->first + " ]");
    DumpClientInfo(info, isClient, it->second);
}

void DataAbilityManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    std::lock_guard<ffrt::mutex> locker(mutex_);

    auto queryInfo = [&info, isPerm](DataAbilityRecordPtrMap::reference data) {
        auto dataAbilityRecord = data.second;
        if (!dataAbilityRecord) {
            return;
        }

        auto abilityRecord = dataAbilityRecord->GetAbilityRecord();
        if (!abilityRecord) {
            return;
        }

        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
            }
        }
    };

    std::for_each(dataAbilityRecordsLoading_.begin(), dataAbilityRecordsLoading_.end(), queryInfo);
    std::for_each(dataAbilityRecordsLoaded_.begin(), dataAbilityRecordsLoaded_.end(), queryInfo);
}

void DataAbilityManager::RestartDataAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    // restart data ability if necessary
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER(bundleMgrHelper);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bool getBundleInfos = bundleMgrHelper->GetBundleInfos(
        OHOS::AppExecFwk::GET_BUNDLE_DEFAULT, bundleInfos, USER_ID_NO_HEAD);
    if (!getBundleInfos) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetBundleInfos failed");
        return;
    }

    for (size_t i = 0; i < bundleInfos.size(); i++) {
        bool keepAliveEnable = bundleInfos[i].isKeepAlive;
        AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(bundleInfos[i].name, keepAliveEnable);
        if (!keepAliveEnable || bundleInfos[i].applicationInfo.process.empty()) {
            continue;
        }
        for (auto hapModuleInfo : bundleInfos[i].hapModuleInfos) {
            if (hapModuleInfo.isModuleJson) {
                // new application model, it cannot be a data ability
                continue;
            }
            // old application model, it maybe a data ability
            std::string mainElement = hapModuleInfo.mainAbility;
            if (abilityRecord->GetAbilityInfo().name != mainElement ||
                abilityRecord->GetAbilityInfo().process != bundleInfos[i].applicationInfo.process) {
                continue;
            }
            std::string uriStr;
            bool getDataAbilityUri = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetDataAbilityUri(
                hapModuleInfo.abilityInfos, mainElement, uriStr);
            if (getDataAbilityUri) {
                TAG_LOGI(AAFwkTag::DATA_ABILITY, "restart dataability: %{public}s, uri: %{public}s",
                    abilityRecord->GetAbilityInfo().name.c_str(), uriStr.c_str());
                Uri uri(uriStr);
                OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->AcquireDataAbility(uri, true, nullptr);
                return;
            }
        }
    }
}

void DataAbilityManager::ReportDataAbilityAcquired(const sptr<IRemoteObject> &client, bool isNotHap,
    std::shared_ptr<DataAbilityRecord> &record)
{
    DataAbilityCaller caller;
    caller.isNotHap = isNotHap;
    caller.callerPid = IPCSkeleton::GetCallingPid();
    caller.callerUid = IPCSkeleton::GetCallingUid();
    caller.callerToken = client;
    if (client && !isNotHap) {
        auto abilityRecord = Token::GetAbilityRecordByToken(client);
        if (abilityRecord) {
            caller.callerName = abilityRecord->GetAbilityInfo().bundleName;
        }
    } else {
        caller.callerName = ConnectionStateManager::GetProcessNameByPid(caller.callerPid);
    }

    DelayedSingleton<ConnectionStateManager>::GetInstance()->AddDataAbilityConnection(caller, record);
}

void DataAbilityManager::ReportDataAbilityReleased(const sptr<IRemoteObject> &client, bool isNotHap,
    std::shared_ptr<DataAbilityRecord> &record)
{
    DataAbilityCaller caller;
    caller.isNotHap = isNotHap;
    caller.callerPid = IPCSkeleton::GetCallingPid();
    caller.callerUid = IPCSkeleton::GetCallingUid();
    caller.callerToken = client;
    DelayedSingleton<ConnectionStateManager>::GetInstance()->RemoveDataAbilityConnection(caller, record);
}
}  // namespace AAFwk
}  // namespace OHOS
