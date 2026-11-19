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

#ifndef OHOS_ABILITY_RUNTIME_KVSTORE_TRANSACTION_H
#define OHOS_ABILITY_RUNTIME_KVSTORE_TRANSACTION_H

#include <memory>

#include "distributed_kv_data_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief RAII wrapper for a SingleKvStore transaction
 *
 * Starts a transaction on construction and rolls it back on destruction
 * unless Commit() or Rollback() has ended it. Guarantees an open transaction
 * is never left behind on early returns or unexpected scope exits.
 *
 * Transactions are not nestable: create at most one wrapper per store at a
 * time and serialize concurrent transactions with the caller's own lock.
 */
class KvStoreTransaction final {
public:
    /**
     * @brief Start a transaction on the given store
     * @param store The SingleKvStore to manage; a null store is accepted and
     *              leaves the transaction un-started (IsStarted() is false)
     */
    explicit KvStoreTransaction(std::shared_ptr<DistributedKv::SingleKvStore> store)
        : store_(std::move(store)), committed_(false), started_(false)
    {
        if (store_) {
            started_ = (store_->StartTransaction() == DistributedKv::Status::SUCCESS);
        }
    }

    ~KvStoreTransaction()
    {
        if (!committed_ && started_ && store_) {
            store_->Rollback();
        }
    }

    /**
     * @brief Commit the transaction
     * @return DistributedKv::Status SUCCESS only if the transaction was
     *         started and committed; on commit failure the transaction is
     *         rolled back before returning the failure status
     */
    DistributedKv::Status Commit()
    {
        if (!started_ || !store_) {
            return DistributedKv::Status::ERROR;
        }
        DistributedKv::Status status = store_->Commit();
        if (status != DistributedKv::Status::SUCCESS) {
            // Commit failed, rollback to clean up
            store_->Rollback();
        }
        committed_ = true;
        return status;
    }

    /**
     * @brief Roll back the transaction and end it; the destructor will not
     *        roll back again
     */
    void Rollback()
    {
        if (started_ && store_) {
            store_->Rollback();
        }
        committed_ = true;
    }

    /**
     * @brief Check whether the transaction was successfully started
     * @return bool true if StartTransaction succeeded on construction
     */
    bool IsStarted() const
    {
        return started_;
    }

    // Disable copy and move
    DISALLOW_COPY_AND_MOVE(KvStoreTransaction);

private:
    std::shared_ptr<DistributedKv::SingleKvStore> store_;
    bool committed_;
    bool started_;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_KVSTORE_TRANSACTION_H
