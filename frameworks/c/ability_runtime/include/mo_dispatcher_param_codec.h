/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_PARAM_CODEC_H
#define ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_PARAM_CODEC_H

#include <unordered_set>

#include "message_parcel.h"
#include "mo_dispatcher_metadata_manager.h"

namespace OHOS::AbilityRuntime {
class ModObjDispatcherParamCodec {
public:
    static AbilityRuntime_ErrorCode MarshalCallRequest(const MoMethodMeta& methodMeta,
        const OH_AbilityRuntime_ModObjDispatcher_InputParams* inputParams, MessageParcel& dataParcel);

    static AbilityRuntime_ErrorCode UnmarshalCallResult(const MoMethodMeta& methodMeta,
        MessageParcel& replyParcel, OH_AbilityRuntime_ModObjDispatcher_Variant* result, int32_t* pMethodErrCode);

private:
    static AbilityRuntime_ErrorCode WriteRawValue(MessageParcel& parcel,
        const std::shared_ptr<MoTypeInfo>& typeInfo, const OH_AbilityRuntime_ModObjDispatcher_Variant* value);

    static AbilityRuntime_ErrorCode ReadRawValue(MessageParcel& parcel,
        const std::shared_ptr<MoTypeInfo>& typeInfo, OH_AbilityRuntime_ModObjDispatcher_Variant* value);

    static AbilityRuntime_ErrorCode WriteRawValueImpl(MessageParcel& parcel,
        const std::shared_ptr<MoTypeInfo>& typeInfo, const OH_AbilityRuntime_ModObjDispatcher_Variant* value,
        std::unordered_set<const void*>& visited);
};
} // namespace OHOS::AbilityRuntime

#endif // ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_PARAM_CODEC_H
