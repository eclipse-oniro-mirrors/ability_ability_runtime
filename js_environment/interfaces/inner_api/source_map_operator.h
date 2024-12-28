/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
#define OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
#include <memory>
#include <set>
#include <string>

#include "ffrt.h"
#include "source_map.h"

namespace OHOS {
namespace JsEnv {
namespace {
enum InitStatus { NOT_EXECUTED, EXECUTED_SUCCESSFULLY };
}
class SourceMapOperator : public std::enable_shared_from_this<SourceMapOperator> {
public:
    SourceMapOperator(const std::string bundleName, bool isModular, bool hasFile)
        : bundleName_(bundleName), isModular_(isModular), hasFile_(hasFile), initStatus_(NOT_EXECUTED) {}

    ~SourceMapOperator() = default;

    void InitSourceMap()
    {
        sourceMapObj_ = std::make_shared<SourceMap>();

        auto init = [weak = weak_from_this()]() {
            auto sourceMapOperator = weak.lock();
            if (sourceMapOperator != nullptr && sourceMapOperator->sourceMapObj_ != nullptr) {
                std::vector<std::string> hapList;
                sourceMapOperator->sourceMapObj_->GetHapPath(sourceMapOperator->bundleName_, hapList);
                for (auto &hapInfo : hapList) {
                    if (!hapInfo.empty()) {
                        sourceMapOperator->sourceMapObj_->Init(sourceMapOperator->isModular_, hapInfo);
                    }
                }
                sourceMapOperator->initStatus_ = EXECUTED_SUCCESSFULLY;
            }
        };

        ffrt::submit(init, {}, {}, ffrt::task_attr().qos(ffrt::qos_user_initiated));
    }

    std::string TranslateBySourceMap(const std::string& stackStr)
    {
        if (hasFile_ && sourceMapObj_ != nullptr) {
            return sourceMapObj_->TranslateBySourceMap(stackStr);
        } else {
            return NOT_FOUNDMAP + stackStr;
        }
    }

    bool TranslateUrlPositionBySourceMap(std::string& url, int& line, int& column, std::string& packageName)
    {
        if (sourceMapObj_ == nullptr) {
            return false;
        }
        return sourceMapObj_->TranslateUrlPositionBySourceMap(url, line, column, packageName);
    }

    bool GetInitStatus() const
    {
        return (initStatus_ == InitStatus::EXECUTED_SUCCESSFULLY);
    }

    void SetInitStatus(InitStatus value)
    {
        initStatus_ = value;
    }

    std::shared_ptr<SourceMap> GetSourceMapObj() const
    {
        return sourceMapObj_;
    }

    bool GetHasFile() const
    {
        return hasFile_;
    }
private:
    const std::string bundleName_;
    bool isModular_ = false;
    bool hasFile_ = false;
    std::shared_ptr<SourceMap> sourceMapObj_;
    InitStatus initStatus_;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
