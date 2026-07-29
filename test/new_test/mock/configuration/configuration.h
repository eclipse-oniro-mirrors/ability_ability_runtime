/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_BASE_CONFIGURATION_H
#define MOCK_OHOS_ABILITY_BASE_CONFIGURATION_H

#include <mutex>
#include <set>
#include <string>
#include <vector>
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
class Configuration final: public Parcelable {
public:
    Configuration();

    void CompareDifferent(std::vector<std::string> &diffKeyV, const Configuration &other) const {}

    void Merge(const std::vector<std::string> &diffKeyV, const Configuration &other) {}

    bool AddItem(int displayId, const std::string &key, const std::string &value)
    {
        return false;
    }

    std::string GetItem(int displayId, const std::string &key) const
    {
        return "";
    }

    int RemoveItem(int displayId, const std::string &key)
    {
        return 0;
    }

    bool AddItem(const std::string &key, const std::string &value)
    {
        return false;
    }

    std::string GetItem(const std::string &key) const
    {
        return "";
    }

    int RemoveItem(const std::string &key)
    {
        return 0;
    }

    int GetItemSize() const
    {
        return 0;
    }

    const std::string GetName() const
    {
        return "";
    }

    bool ReadFromParcel(Parcel &parcel)
    {
        return false;
    }

    virtual bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }

    static Configuration *Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }

    void FilterDuplicates(const Configuration &other) {}
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_CONFIGURATION_H
