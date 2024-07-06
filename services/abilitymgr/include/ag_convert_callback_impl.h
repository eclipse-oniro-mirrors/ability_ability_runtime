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

#ifndef OHOS_ABILITY_RUNTIME_AG_CONVERT_CALLBACK_IMPL_H
#define OHOS_ABILITY_RUNTIME_AG_CONVERT_CALLBACK_IMPL_H

#include "convert_callback_stub.h"

#include <functional>

namespace OHOS {
namespace AAFwk {
using ConvertCallbackTask = std::function<void(int, AAFwk::Want&)>;

/**
 * @class ConvertCallbackImpl the implementation of the IConvertCallback
*/
class ConvertCallbackImpl : public OHOS::AppDomainVerify::ConvertCallbackStub {
public:
    explicit ConvertCallbackImpl(ConvertCallbackTask&& task) : task_(task) {}
    virtual ~ConvertCallbackImpl() = default;

    void OnConvert(int resultCode, AAFwk::Want& want) override;

private:
    ConvertCallbackTask task_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_AG_CONVERT_CALLBACK_IMPL_H