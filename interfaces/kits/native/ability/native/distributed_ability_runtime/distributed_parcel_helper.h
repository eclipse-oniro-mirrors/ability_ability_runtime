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

#ifndef OHOS_ABILITY_RUNTIME_DISTRIBUTED_PARCEL_HELPER_H
#define OHOS_ABILITY_RUNTIME_DISTRIBUTED_PARCEL_HELPER_H

#include <cinttypes>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
#define PARCEL_WRITE_HELPER(parcel, type, value) \
    do { \
        bool ret = parcel.Write##type((value)); \
        if (!ret) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "write value failed"); \
            return ERR_FLATTEN_OBJECT; \
        } \
    } while (0)

#define PARCEL_WRITE_HELPER_NORET(parcel, type, value) \
    do { \
        bool ret = parcel.Write##type((value)); \
        if (!ret) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "write value failed"); \
            return; \
        } \
    } while (0)

#define PARCEL_WRITE_HELPER_RET(parcel, type, value, failRet) \
    do { \
        bool ret = parcel.Write##type((value)); \
        if (!ret) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "write value failed"); \
            return failRet; \
        } \
    } while (0)

#define PARCEL_READ_HELPER(parcel, type, out) \
    do { \
        bool ret = parcel.Read##type((out)); \
        if (!ret) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "read value failed"); \
            return ERR_FLATTEN_OBJECT; \
        } \
    } while (0)

#define PARCEL_READ_HELPER_RET(parcel, type, out, failRet) \
    do { \
        bool ret = parcel.Read##type((out)); \
        if (!ret) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "read value failed"); \
            return failRet; \
        } \
    } while (0)

#define PARCEL_READ_HELPER_NORET(parcel, type, out) \
    do { \
        bool ret = parcel.Read##type((out)); \
        if (!ret) { \
            TAG_LOGW(AAFwkTag::DISTRIBUTED, "read value failed"); \
        } \
    } while (0)

#define PARCEL_TRANSACT_SYNC_RET_INT(remote, code, data, reply) \
    do { \
        MessageOption option; \
        int32_t error = remote->SendRequest(code, data, reply, option); \
        if (error != ERR_NONE) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "error: %{public}d", error); \
            return error; \
        } \
        int32_t result = reply.ReadInt32(); \
        return result; \
    } while (0)

#define PARCEL_TRANSACT_SYNC_NORET(remote, code, data, reply) \
    do { \
        MessageOption option; \
        int32_t result = remote->SendRequest(code, data, reply, option); \
        if (result != ERR_NONE) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "result: %{public}d", result); \
            return; \
        } \
    } while (0)

#define PARCEL_TRANSACT_ASYNC_RET_INT(remote, code, data, reply) \
    do { \
        MessageOption option(MessageOption::TF_ASYNC); \
        int32_t error = remote->SendRequest(code, data, reply, option); \
        if (error != ERR_NONE) { \
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "error: %{public}d", error); \
            return error; \
        } \
        return ERR_OK; \
    } while (0)

#define PARCEL_WRITE_REPLY_NOERROR(reply, type, result) \
    do { \
        bool ret = reply.Write##type(result); \
        if (!ret) { \
            TAG_LOGW(AAFwkTag::DISTRIBUTED, "write reply failed"); \
        } \
        return ERR_NONE; \
    } while (0)
} // namespace AAFwk
} // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_DISTRIBUTED_PARCEL_HELPER_H */
