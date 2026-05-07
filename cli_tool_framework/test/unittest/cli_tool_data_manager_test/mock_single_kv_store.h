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
#ifndef MOCK_SINGLE_KV_STORE_H
#define MOCK_SINGLE_KV_STORE_H

#define private public
#define protected public
#include "distributed_kv_data_manager.h"
#include "kvstore.h"
#undef private
#undef protected

namespace OHOS {
namespace CliTool {
class MockSingleKvStore : public DistributedKv::SingleKvStore {
public:
    MockSingleKvStore() {};

    virtual ~MockSingleKvStore() {};

    DistributedKv::Status Get(const DistributedKv::Key &key, DistributedKv::Value &value) override
    {
        if (Get_ != DistributedKv::Status::SUCCESS) {
            return Get_;
        }
        if (mockData_.find(key.ToString()) != mockData_.end()) {
            value = mockData_[key.ToString()];
            return DistributedKv::Status::SUCCESS;
        }
        return DistributedKv::Status::KEY_NOT_FOUND;
    };

    DistributedKv::Status GetEntries(
        const DistributedKv::Key &prefix, std::vector<DistributedKv::Entry> &entries) const override
    {
        return GetEntries_;
    };

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

    DistributedKv::Status GetResultSet(const DistributedKv::DataQuery &query,
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

    DistributedKv::Status Sync(const std::vector<std::string> &devices, DistributedKv::SyncMode mode,
        const DistributedKv::DataQuery &query,
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

    DistributedKv::Status SetCapabilityRange(
        const std::vector<std::string> &localLabels, const std::vector<std::string> &remoteLabels) const override
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

    DistributedKv::Status SetIdentifier(const std::string &accountId, const std::string &appId,
        const std::string &storeId, const std::vector<std::string> &tagretDev) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::StoreId GetStoreId() const override
    {
        DistributedKv::StoreId storeId;
        storeId.storeId = "cli_tools_store";
        return storeId;
    };

    DistributedKv::Status Put(const DistributedKv::Key &key, const DistributedKv::Value &value) override
    {
        if (Put_ != DistributedKv::Status::SUCCESS) {
            return Put_;
        }
        mockData_[key.ToString()] = value;
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status PutBatch(const std::vector<DistributedKv::Entry> &entries) override
    {
        return DistributedKv::Status::SUCCESS;
    };

    DistributedKv::Status Delete(const DistributedKv::Key &key) override
    {
        if (Delete_ != DistributedKv::Status::SUCCESS) {
            return Delete_;
        }
        mockData_.erase(key.ToString());
        return DistributedKv::Status::SUCCESS;
    };

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

    void SetMockData(const std::string &key, const std::string &value)
    {
        mockData_[key] = DistributedKv::Value(value);
    }

    DistributedKv::Status GetEntries_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Status Delete_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Status Put_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Status Get_ = DistributedKv::Status::SUCCESS;

private:
    std::map<std::string, DistributedKv::Value> mockData_;
};
} // namespace CliTool
} // namespace OHOS
#endif
