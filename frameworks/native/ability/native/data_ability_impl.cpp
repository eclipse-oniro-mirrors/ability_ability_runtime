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

#include "data_ability_impl.h"

#include "abs_shared_result_set.h"
#include "accesstoken_kit.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string READ = "r";
const std::string WRITE = "w";
}
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
using OHOS::Security::AccessToken::AccessTokenKit;
void DataAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY,
        "sourceState:%{public}d; targetState:%{public}d; isNewWant:%{public}d", lifecycleState_,
        targetState.state, targetState.isNewWant);
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "lifeCycleState equals");
        return;
    }

    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_ACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
                SetUriString(targetState.caller.deviceId + "/" + targetState.caller.bundleName + "/" +
                             targetState.caller.abilityName);
                Start(want);
            } else {
                return;
            }
            break;
        }
        default: {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "state error");
            return;
        }
    }

    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> DataAbilityImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> types;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return types;
    }

    types = ability_->GetFileTypes(uri, mimeTypeFilter);
    return types;
}

/**
 * @brief Opens a file in a specified remote path.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 *  or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the file descriptor.
 */
int DataAbilityImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return fd;
    }

    if (!CheckOpenFilePermission(mode)) {
        return fd;
    }

    fd = ability_->OpenFile(uri, mode);
    return fd;
}

/**
 * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
 * inside of their .hap.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
 * data, or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the RawFileDescriptor object containing file descriptor.
 */
int DataAbilityImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return fd;
    }

    if (!CheckOpenFilePermission(mode)) {
        return fd;
    }

    fd = ability_->OpenRawFile(uri, mode);
    return fd;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int DataAbilityImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return index;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return index;
    }

    index = ability_->Insert(uri, value);
    return index;
}

/**
 * @brief Updates data records in the database.
 *
 * @param uri Indicates the path of data to update.
 * @param value Indicates the data to update. This parameter can be null.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records updated.
 */
int DataAbilityImpl::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return index;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return index;
    }

    index = ability_->Update(uri, value, predicates);
    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records deleted.
 */
int DataAbilityImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return index;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return index;
    }

    index = ability_->Delete(uri, predicates);
    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of data to query.
 * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the query result.
 */
std::shared_ptr<NativeRdb::AbsSharedResultSet> DataAbilityImpl::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return nullptr;
    }

    if (!CheckReadAndWritePermission(READ)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
        return nullptr;
    }

    return ability_->Query(uri, columns, predicates);
}

std::shared_ptr<AppExecFwk::PacMap> DataAbilityImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return nullptr;
    }

    return ability_->Call(uri, method, arg, pacMap);
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string DataAbilityImpl::GetType(const Uri &uri)
{
    std::string type;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return type;
    }
    type = ability_->GetType(uri);
    return type;
}

/**
 * @brief Reloads data in the database.
 *
 * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
 * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
 * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
 * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
 *
 * @return Returns true if the data is successfully reloaded; returns false otherwise.
 */
bool DataAbilityImpl::Reload(const Uri &uri, const PacMap &extras)
{
    bool ret = false;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return ret;
    }
    ret = ability_->Reload(uri, extras);
    return ret;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int DataAbilityImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return ret;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return ret;
    }

    ret = ability_->BatchInsert(uri, values);
    return ret;
}
/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the Data ability must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this Data ability does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri DataAbilityImpl::NormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return urivalue;
    }

    if (!CheckReadAndWritePermission(READ)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
        return urivalue;
    }

    urivalue = ability_->NormalizeUri(uri);
    return urivalue;
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the Data ability must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this Data ability does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri DataAbilityImpl::DenormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return urivalue;
    }

    if (!CheckReadAndWritePermission(READ)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "no read permission");
        return urivalue;
    }

    urivalue = ability_->DenormalizeUri(uri);
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        results.clear();
        return results;
    }

    if (!CheckExecuteBatchPermission(operations)) {
        results.clear();
        return results;
    }

    results = ability_->ExecuteBatch(operations);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "end, results size:%{public}zu", results.size());
    return results;
}

bool DataAbilityImpl::CheckExecuteBatchPermission(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations) const
{
    bool needCheckReadPermission = false;
    bool needCheckWritePermission = false;

    size_t size = operations.size();
    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<DataAbilityOperation> operation = operations[i];
        if (operation == nullptr) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "null operation, index: %{public}d", static_cast<int32_t>(i));
            return false;
        }
        if (operation->IsInsertOperation() || operation->IsUpdateOperation() || operation->IsDeleteOperation()) {
            needCheckWritePermission = true;
        } else if (operation->IsAssertOperation()) {
            needCheckReadPermission = true;
        }
        if (needCheckReadPermission && needCheckWritePermission) {
            break;
        }
    }

    if (needCheckReadPermission) {
        if (!CheckReadAndWritePermission(READ)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
            return false;
        }
    }

    if (needCheckWritePermission) {
        if (!CheckReadAndWritePermission(WRITE)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
            return false;
        }
    }

    return true;
}

bool DataAbilityImpl::CheckOpenFilePermission(const std::string &mode) const
{
    if (mode.find(READ) != string::npos) {
        if (!CheckReadAndWritePermission(READ)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
            return false;
        }
    }
    if (mode.find(WRITE) != string::npos) {
        if (!CheckReadAndWritePermission(WRITE)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
            return false;
        }
    }

    return true;
}

bool DataAbilityImpl::CheckReadAndWritePermission(const std::string &permissionType) const
{
    std::string permission = GetPermissionInfo(permissionType);
    if (permission.empty()) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "empty permission");
        return true;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    int checkReadPermission = AccessTokenKit::VerifyAccessToken(tokenId, permission);
    if (checkReadPermission != ERR_OK) {
        std::shared_ptr<AbilityInfo> abilityInfo = ability_->GetAbilityInfo();
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "no permission, bundleName: %{public}s", abilityInfo->bundleName.c_str());
        return false;
    }

    return true;
}

std::string DataAbilityImpl::GetPermissionInfo(const std::string &permissionType) const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "permissionType:%{public}s", permissionType.c_str());
    std::shared_ptr<AbilityInfo> abilityInfo = ability_->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "null abilityInfo");
        return "";
    }
    if (permissionType == READ) {
        return abilityInfo->readPermission;
    } else if (permissionType == WRITE) {
        return abilityInfo->writePermission;
    } else {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "invalid permissionType:%{public}s", permissionType.c_str());
        return "";
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
