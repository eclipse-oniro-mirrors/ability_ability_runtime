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

#include "child_process_request.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool ChildProcessRequest::ReadFromParcel(Parcel &parcel)
{
    std::u16string srcEntryTemp;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, srcEntryTemp);
    srcEntry = Str16ToStr8(srcEntryTemp);

    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, childProcessType);
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, childProcessCount);
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isStartWithDebug);
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isStaticChildProcess);

    std::unique_ptr<ChildProcessArgs> argsRead(parcel.ReadParcelable<ChildProcessArgs>());
    if (!argsRead) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read ChildProcessArgs failed.");
        return false;
    }
    args = *argsRead;

    std::unique_ptr<ChildProcessOptions> optionsRead(parcel.ReadParcelable<ChildProcessOptions>());
    if (!optionsRead) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read ChildProcessOptions failed.");
        return false;
    }
    options = *optionsRead;

    return true;
}

ChildProcessRequest *ChildProcessRequest::Unmarshalling(Parcel &parcel)
{
    ChildProcessRequest *data = new (std::nothrow) ChildProcessRequest();
    if (data && !data->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "Read from parcel failed.");
        delete data;
        data = nullptr;
    }
    return data;
}

bool ChildProcessRequest::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(srcEntry));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(childProcessType));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(childProcessCount));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isStartWithDebug);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, isStaticChildProcess);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &args);
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &options);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
