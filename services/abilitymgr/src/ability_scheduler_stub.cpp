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

#include "ability_scheduler_stub.h"

#include "ability_manager_errors.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_tag_wrapper.h"
#include "ishared_result_set.h"
#include "session_info.h"
#include "values_bucket.h"

namespace OHOS {
namespace AAFwk {
constexpr int CYCLE_LIMIT = 2000;
AbilitySchedulerStub::AbilitySchedulerStub()
{}

AbilitySchedulerStub::~AbilitySchedulerStub()
{}

int AbilitySchedulerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AbilitySchedulerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "descriptor not equal to remote");
        return ERR_INVALID_STATE;
    }
    return OnRemoteRequestInner(code, data, reply, option);
}

int AbilitySchedulerStub::OnRemoteRequestInner(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = OnRemoteRequestInnerFirst(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSecond(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerThird(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AbilitySchedulerStub::OnRemoteRequestInnerFirst(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case SCHEDULE_ABILITY_TRANSACTION:
            return AbilityTransactionInner(data, reply);
        case SEND_RESULT:
            return SendResultInner(data, reply);
        case SCHEDULE_ABILITY_CONNECT:
            return ConnectAbilityInner(data, reply);
        case SCHEDULE_ABILITY_DISCONNECT:
            return DisconnectAbilityInner(data, reply);
        case SCHEDULE_ABILITY_COMMAND:
            return CommandAbilityInner(data, reply);
        case SCHEDULE_ABILITY_PREPARE_TERMINATE:
            return PrepareTerminateAbilityInner(data, reply);
        case SCHEDULE_ABILITY_COMMAND_WINDOW:
            return CommandAbilityWindowInner(data, reply);
        case SCHEDULE_SAVE_ABILITY_STATE:
            return SaveAbilityStateInner(data, reply);
        case SCHEDULE_RESTORE_ABILITY_STATE:
            return RestoreAbilityStateInner(data, reply);
        case SCHEDULE_GETFILETYPES:
            return GetFileTypesInner(data, reply);
        case SCHEDULE_OPENFILE:
            return OpenFileInner(data, reply);
        case SCHEDULE_OPENRAWFILE:
            return OpenRawFileInner(data, reply);
        case SCHEDULE_INSERT:
            return InsertInner(data, reply);
        case SCHEDULE_UPDATE:
            return UpdatetInner(data, reply);
        case SCHEDULE_DELETE:
            return DeleteInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilitySchedulerStub::OnRemoteRequestInnerSecond(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case SCHEDULE_QUERY:
            return QueryInner(data, reply);
        case SCHEDULE_CALL:
            return CallInner(data, reply);
        case SCHEDULE_GETTYPE:
            return GetTypeInner(data, reply);
        case SCHEDULE_RELOAD:
            return ReloadInner(data, reply);
        case SCHEDULE_BATCHINSERT:
            return BatchInsertInner(data, reply);
        case SCHEDULE_REGISTEROBSERVER:
            return RegisterObserverInner(data, reply);
        case SCHEDULE_UNREGISTEROBSERVER:
            return UnregisterObserverInner(data, reply);
        case SCHEDULE_NOTIFYCHANGE:
            return NotifyChangeInner(data, reply);
        case SCHEDULE_NORMALIZEURI:
            return NormalizeUriInner(data, reply);
        case SCHEDULE_DENORMALIZEURI:
            return DenormalizeUriInner(data, reply);
        case SCHEDULE_EXECUTEBATCH:
            return ExecuteBatchInner(data, reply);
        case NOTIFY_CONTINUATION_RESULT:
            return NotifyContinuationResultInner(data, reply);
        case REQUEST_CALL_REMOTE:
            return CallRequestInner(data, reply);
        case CONTINUE_ABILITY:
            return ContinueAbilityInner(data, reply);
        case DUMP_ABILITY_RUNNER_INNER:
            return DumpAbilityInfoInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilitySchedulerStub::OnRemoteRequestInnerThird(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case SCHEDULE_SHARE_DATA:
            return ShareDataInner(data, reply);
        case SCHEDULE_ONEXECUTE_INTENT:
            return OnExecuteIntentInner(data, reply);
        case SCHEDULE_EXECUTE_SKILL:
            return ExecuteSkillInner(data, reply);
        case CREATE_MODAL_UI_EXTENSION:
            return CreateModalUIExtensionInner(data, reply);
        case UPDATE_SESSION_TOKEN:
            return UpdateSessionTokenInner(data, reply);
        case SCHEDULE_COLLABORATE_DATA:
            return CollaborateDataInner(data);
        case SCHEDULE_ABILITY_REQUEST_FAILURE:
            return ScheduleAbilityRequestFailureInner(data);
        case SCHEDULE_ABILITY_REQUEST_SUCCESS:
            return ScheduleAbilityRequestSuccessInner(data);
        case SCHEDULE_ABILITIES_REQUEST_DONE:
            return ScheduleAbilitiesRequestDoneInner(data);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilitySchedulerStub::AbilityTransactionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<LifeCycleStateInfo> stateInfo(data.ReadParcelable<LifeCycleStateInfo>());
    if (!stateInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ReadParcelable<LifeCycleStateInfo> failed");
        return ERR_INVALID_VALUE;
    }
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    ScheduleAbilityTransaction(*want, *stateInfo, sessionInfo);
    return NO_ERROR;
}

int AbilitySchedulerStub::ShareDataInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t requestCode = data.ReadInt32();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "requestCode:%{public}d", requestCode);
    ScheduleShareData(requestCode);
    return NO_ERROR;
}

int AbilitySchedulerStub::SendResultInner(MessageParcel &data, MessageParcel &reply)
{
    int requestCode = data.ReadInt32();
    int resultCode = data.ReadInt32();
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    SendResult(requestCode, resultCode, *want);
    return NO_ERROR;
}

int AbilitySchedulerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    ScheduleConnectAbility(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    ScheduleDisconnectAbility(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::CommandAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    bool reStart = data.ReadBool();
    int startId = data.ReadInt32();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ReadInt32, startId:%{public}d", startId);
    ScheduleCommandAbility(*want, reStart, startId);
    return NO_ERROR;
}

int AbilitySchedulerStub::PrepareTerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "prepare terminate call");
    bool ret = SchedulePrepareTerminateAbility();
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to write ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::CommandAbilityWindowInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    sptr<SessionInfo> sessionInfo(data.ReadParcelable<SessionInfo>());
    int32_t winCmd = data.ReadInt32();
    ScheduleCommandAbilityWindow(*want, sessionInfo, static_cast<WindowCommand>(winCmd));
    return NO_ERROR;
}

int AbilitySchedulerStub::SaveAbilityStateInner(MessageParcel &data, MessageParcel &reply)
{
    ScheduleSaveAbilityState();
    return NO_ERROR;
}

int AbilitySchedulerStub::RestoreAbilityStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<PacMap> pacMap(data.ReadParcelable<PacMap>());
    if (pacMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null pacMap");
        return ERR_INVALID_VALUE;
    }
    ScheduleRestoreAbilityState(*pacMap);
    return NO_ERROR;
}

int AbilitySchedulerStub::GetFileTypesInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::string mimeTypeFilter = data.ReadString();
    if (mimeTypeFilter.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null mimeTypeFilter");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> types = GetFileTypes(*uri, mimeTypeFilter);
    if (!reply.WriteStringVector(types)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteStringVector types");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::OpenFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::string mode = data.ReadString();
    if (mode.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null mode");
        return ERR_INVALID_VALUE;
    }
    int fd = OpenFile(*uri, mode);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "openFile fail, fd: %{public}d", fd);
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteFileDescriptor(fd)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteFileDescriptor fd");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::OpenRawFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::string mode = data.ReadString();
    if (mode.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null mode");
        return ERR_INVALID_VALUE;
    }
    int fd = OpenRawFile(*uri, mode);
    if (!reply.WriteInt32(fd)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 fd");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::InsertInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    int index = Insert(*uri, NativeRdb::ValuesBucket::Unmarshalling(data));
    if (!reply.WriteInt32(index)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end");
    return NO_ERROR;
}

int AbilitySchedulerStub::CallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::string method = data.ReadString();
    if (method.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null method");
        return ERR_INVALID_VALUE;
    }
    std::string arg = data.ReadString();
    if (arg.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null arg");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AppExecFwk::PacMap> pacMap(data.ReadParcelable<AppExecFwk::PacMap>());
    if (pacMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null pacMap");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<AppExecFwk::PacMap> result = Call(*uri, method, arg, *pacMap);
    if (!reply.WriteParcelable(result.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable pacMap error");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end");
    return NO_ERROR;
}

int AbilitySchedulerStub::UpdatetInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    auto value = NativeRdb::ValuesBucket::Unmarshalling(data);
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null predicates");
        return ERR_INVALID_VALUE;
    }
    int index = Update(*uri, std::move(value), *predicates);
    if (!reply.WriteInt32(index)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::DeleteInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null predicates");
        return ERR_INVALID_VALUE;
    }
    int index = Delete(*uri, *predicates);
    if (!reply.WriteInt32(index)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::QueryInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> columns;
    if (!data.ReadStringVector(&columns)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadStringVector columns");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null predicates");
        return ERR_INVALID_VALUE;
    }
    auto resultSet = Query(*uri, columns, *predicates);
    if (resultSet == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null resultSet");
        return ERR_INVALID_VALUE;
    }
    auto result = NativeRdb::ISharedResultSet::WriteToParcel(std::move(resultSet), reply);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null result");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end");
    return NO_ERROR;
}

int AbilitySchedulerStub::GetTypeInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    std::string type = GetType(*uri);
    if (!reply.WriteString(type)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteString type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::ReloadInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<PacMap> extras(data.ReadParcelable<PacMap>());
    if (extras == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null extras");
        return ERR_INVALID_VALUE;
    }
    bool ret = Reload(*uri, *extras);
    if (!reply.WriteBool(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to writeBool ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::BatchInsertInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }

    int count = 0;
    if (!data.ReadInt32(count)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 index");
        return ERR_INVALID_VALUE;
    }

    if (count > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "count too large");
        return ERR_INVALID_VALUE;
    }
    std::vector<NativeRdb::ValuesBucket> values;
    for (int i = 0; i < count; i++) {
        values.emplace_back(NativeRdb::ValuesBucket::Unmarshalling(data));
    }

    int ret = BatchInsert(*uri, values);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    auto obServer = iface_cast<IDataAbilityObserver>(data.ReadRemoteObject());
    if (obServer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null obServer");
        return ERR_INVALID_VALUE;
    }

    bool ret = ScheduleRegisterObserver(*uri, obServer);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::UnregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }
    auto obServer = iface_cast<IDataAbilityObserver>(data.ReadRemoteObject());
    if (obServer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null obServer");
        return ERR_INVALID_VALUE;
    }

    bool ret = ScheduleUnregisterObserver(*uri, obServer);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::NotifyChangeInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }

    bool ret = ScheduleNotifyChange(*uri);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::NormalizeUriInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }

    Uri ret("");
    ret = NormalizeUri(*uri);
    if (!reply.WriteParcelable(&ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::DenormalizeUriInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null uri");
        return ERR_INVALID_VALUE;
    }

    Uri ret("");
    ret = DenormalizeUri(*uri);
    if (!reply.WriteParcelable(&ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::ExecuteBatchInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    int count = 0;
    if (!data.ReadInt32(count)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 count");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "count:%{public}d", count);
    if (count > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "count too large");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> operations;
    for (int i = 0; i < count; i++) {
        std::shared_ptr<AppExecFwk::DataAbilityOperation> dataAbilityOperation(
            data.ReadParcelable<AppExecFwk::DataAbilityOperation>());
        if (dataAbilityOperation == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null dataAbilityOperation, index: %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        operations.push_back(dataAbilityOperation);
    }

    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results = ExecuteBatch(operations);
    int total = (int)results.size();
    if (!reply.WriteInt32(total)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "total:%{public}d", total);
    for (int i = 0; i < total; i++) {
        if (results[i] == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "null results[i], index: %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        if (!reply.WriteParcelable(results[i].get())) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "fail to WriteParcelable operation, index: %{public}d", i);
            return ERR_INVALID_VALUE;
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end");
    return NO_ERROR;
}

int AbilitySchedulerStub::ContinueAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    uint32_t versionCode = data.ReadUint32();
    ContinueAbility(deviceId, versionCode);
    return NO_ERROR;
}

int AbilitySchedulerStub::NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = data.ReadInt32();
    NotifyContinuationResult(result);
    return NO_ERROR;
}

int AbilitySchedulerStub::DumpAbilityInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> infos;
    std::vector<std::string> params;
    if (!data.ReadStringVector(&params)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "DumpAbilityInfoInner read params error");
        return ERR_INVALID_VALUE;
    }

    DumpAbilityInfo(params, infos);

    return NO_ERROR;
}

int AbilitySchedulerStub::CallRequestInner(MessageParcel &data, MessageParcel &reply)
{
    CallRequest();
    return NO_ERROR;
}

int AbilitySchedulerStub::OnExecuteIntentInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::INTENT, "on execute intent stub");
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    OnExecuteIntent(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::ExecuteSkillInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "execute skill stub");
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    ExecuteSkill(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::CreateModalUIExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    int ret = CreateModalUIExtension(*want);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::UpdateSessionTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> sessionToken = data.ReadRemoteObject();
    UpdateSessionToken(sessionToken);
    return NO_ERROR;
}

int AbilitySchedulerStub::CollaborateDataInner(MessageParcel &data)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    ScheduleCollaborate(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::ScheduleAbilityRequestFailureInner(MessageParcel &data)
{
    std::string requestId = data.ReadString();
    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null element");
        return ERR_INVALID_VALUE;
    }
    std::string message = data.ReadString();
    int32_t resultCode = data.ReadInt32();
    ScheduleAbilityRequestFailure(requestId, *element, message, resultCode);
    return NO_ERROR;
}

int AbilitySchedulerStub::ScheduleAbilityRequestSuccessInner(MessageParcel &data)
{
    std::string requestId = data.ReadString();
    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null element");
        return ERR_INVALID_VALUE;
    }
    ScheduleAbilityRequestSuccess(requestId, *element);
    return NO_ERROR;
}

int AbilitySchedulerStub::ScheduleAbilitiesRequestDoneInner(MessageParcel &data)
{
    std::string requestKey = data.ReadString();
    int32_t resultCode = data.ReadInt32();
    ScheduleAbilitiesRequestDone(requestKey, resultCode);
    return NO_ERROR;
}

void AbilitySchedulerRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "call");

    if (handler_) {
        handler_(remote);
    }
}

AbilitySchedulerRecipient::AbilitySchedulerRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

AbilitySchedulerRecipient::~AbilitySchedulerRecipient()
{}
}  // namespace AAFwk
}  // namespace OHOS
