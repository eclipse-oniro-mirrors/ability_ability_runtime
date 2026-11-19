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
#ifndef UI_ABILITY_LIFECYCLE_MANAGER_THIRD_TEST_MOCK_SINGLE_KV_STORE_H
#define UI_ABILITY_LIFECYCLE_MANAGER_THIRD_TEST_MOCK_SINGLE_KV_STORE_H

#include <gmock/gmock.h>
#include <map>
#include <string>
#include <vector>

#include "distributed_kv_data_manager.h"
#include "kvstore.h"

namespace OHOS {

class MockSingleKvStore : public DistributedKv::SingleKvStore {
public:
    MockSingleKvStore() {};
    virtual ~MockSingleKvStore() {};

    MOCK_METHOD2(Get, DistributedKv::Status(const DistributedKv::Key &key, DistributedKv::Value &value));

    MOCK_CONST_METHOD2(GetEntries,
        DistributedKv::Status(const DistributedKv::Key &prefix, std::vector<DistributedKv::Entry> &entries));

    DistributedKv::Status GetEntries(
        const DistributedKv::DataQuery &query, std::vector<DistributedKv::Entry> &entries) const override
    {
        return GetEntries_;
    };

    DistributedKv::Status GetResultSet(
        const DistributedKv::Key &prefix, std::shared_ptr<DistributedKv::KvStoreResultSet> &resultSet) const override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status GetResultSet(
        const DistributedKv::DataQuery &query,
        std::shared_ptr<DistributedKv::KvStoreResultSet> &resultSet) const override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status CloseResultSet(std::shared_ptr<DistributedKv::KvStoreResultSet> &resultSet) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status GetCount(const DistributedKv::DataQuery &query, int &count) const override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status RemoveDeviceData(const std::string &device) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status GetSecurityLevel(DistributedKv::SecurityLevel &secLevel) const override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Sync(
        const std::vector<std::string> &devices, DistributedKv::SyncMode mode, uint32_t delay) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Sync(
        const std::vector<std::string> &devices, DistributedKv::SyncMode mode, const DistributedKv::DataQuery &query,
        std::shared_ptr<DistributedKv::KvStoreSyncCallback> syncCallback) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status RegisterSyncCallback(std::shared_ptr<DistributedKv::KvStoreSyncCallback> callback) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status UnRegisterSyncCallback() override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status SetSyncParam(const DistributedKv::KvSyncParam &syncParam) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status GetSyncParam(DistributedKv::KvSyncParam &syncParam) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status SetCapabilityEnabled(bool enabled) const override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status SetCapabilityRange(const std::vector<std::string> &localLabels,
        const std::vector<std::string> &remoteLabels) const override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status SubscribeWithQuery(
        const std::vector<std::string> &devices, const DistributedKv::DataQuery &query) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status UnsubscribeWithQuery(
        const std::vector<std::string> &devices, const DistributedKv::DataQuery &query) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::StoreId GetStoreId() const override
    {
        DistributedKv::StoreId storeId;
        storeId.storeId = "";
        return storeId;
    };

    DistributedKv::Status Put(const DistributedKv::Key &key, const DistributedKv::Value &value) override
    {
        putCallTimes_++;
        return Put_;
    };

    DistributedKv::Status PutBatch(const std::vector<DistributedKv::Entry> &entries) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    MOCK_METHOD1(Delete, DistributedKv::Status(const DistributedKv::Key &key));

    DistributedKv::Status DeleteBatch(const std::vector<DistributedKv::Key> &keys) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status StartTransaction() override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Commit() override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Rollback() override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status SubscribeKvStore(
        DistributedKv::SubscribeType type, std::shared_ptr<DistributedKv::KvStoreObserver> observer) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status UnSubscribeKvStore(
        DistributedKv::SubscribeType type, std::shared_ptr<DistributedKv::KvStoreObserver> observer) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Backup(const std::string &file, const std::string &baseDir) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Restore(const std::string &file, const std::string &baseDir) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status DeleteBackup(const std::vector<std::string> &files, const std::string &baseDir,
        std::map<std::string, DistributedKv::Status> &status) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status GetEntries_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Status Put_ = DistributedKv::Status::SUCCESS;
    int32_t putCallTimes_ = 0;
};
} // namespace OHOS
#endif // UI_ABILITY_LIFECYCLE_MANAGER_THIRD_TEST_MOCK_SINGLE_KV_STORE_H
