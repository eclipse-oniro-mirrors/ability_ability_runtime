/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef I_STORAGE_MANAGER_SERVICE_H
#define I_STORAGE_MANAGER_SERVICE_H

#include <sys/types.h>
#include <string>
#include <vector>

namespace OHOS {
namespace StorageManager {
class IStorageManager {
public:
    IStorageManager() {}

    virtual ~IStorageManager() {}

    virtual int32_t PrepareAddUser(int32_t userId, uint32_t flags) override
    {
        return 0;
    }

    virtual int32_t RemoveUser(int32_t userId, uint32_t flags) override
    {
        return 0;
    }

    virtual int32_t PrepareStartUser(int32_t userId) override
    {
        return 0;
    }

    virtual int32_t StopUser(int32_t userId) override
    {
        return 0;
    }

    virtual int32_t CompleteAddUser(int32_t userId) override
    {
        return 0;
    }

    virtual int32_t GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize) override
    {
        return 0;
    }

    virtual int32_t GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize) override
    {
        return 0;
    }

    virtual int32_t SetDirEncryptionPolicy(uint32_t userId,
        const std::string &dirPath, uint32_t level) override
    {
        return 0;
    }

    virtual int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats,
        int32_t index, uint32_t statFlag) override
    {
        return 0;
    }

    virtual int32_t GetSystemSize(int64_t &systemSize) override
    {
        return 0;
    }

    virtual int32_t GetTotalSize(int64_t &totalSize) override
    {
        return 0;
    }

    virtual int32_t GetFreeSize(int64_t &freeSize) override
    {
        return 0;
    }

    virtual int32_t GetUserStorageStats(StorageStats &storageStats) override
    {
        return 0;
    }

    virtual int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats) override
    {
        return 0;
    }

    virtual int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag) override
    {
        return 0;
    }

    virtual int32_t DeleteUserKeys(uint32_t userId) override
    {
        return 0;
    }

    virtual int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) override
    {
        return 0;
    }

    virtual int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                 const std::vector<uint8_t> &newSecret,
                                                 uint64_t secureUid,
                                                 uint32_t userId,
                                                 const std::vector<std::vector<uint8_t>> &plainText) override
    {
        return 0;
    }

    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) override
    {
        return 0;
    }

    virtual int32_t InactiveUserKey(uint32_t userId) override
    {
        return 0;
    }

    virtual int32_t LockUserScreen(uint32_t userId) override
    {
        return 0;
    }

    virtual int32_t UnlockUserScreen(uint32_t userId,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret) override
    {
        return 0;
    }

    virtual int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus) override
    {
        return 0;
    }

    virtual int32_t GenerateAppkey(uint32_t hashId, uint32_t userId,
                                   std::string &keyId, bool needReset = false) override
    {
        return 0;
    }

    virtual int32_t DeleteAppkey(const std::string &keyId) override
    {
        return 0;
    }

    virtual int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount) override
    {
        return 0;
    }

    virtual int32_t CreateRecoverKey(uint32_t userId,
                                     uint32_t userType,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret) override
    {
        return 0;
    }

    virtual int32_t SetRecoverKey(const std::vector<uint8_t> &key) override
    {
        return 0;
    }

    virtual int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType,
                                               const std::vector<uint8_t> &key) override
    {
        return 0;
    }

    virtual int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false) override
    {
        return 0;
    }

    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive) override
    {
        return 0;
    }

    virtual int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override
    {
        return 0;
    }

    virtual int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override
    {
        return 0;
    }

    virtual int32_t CreateShareFile(const StorageFileRawData &storageShareRawData,
        uint32_t tokenId, uint32_t flag, std::vector<int32_t> &funcResult) override
    {
        return 0;
    }

    virtual int32_t DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData) override
    {
        return 0;
    }

    virtual int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats,
        const std::string &type) override
    {
        return 0;
    }

    virtual int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize) override
    {
        return 0;
    }

    virtual int32_t MountMediaFuse(int32_t userId, int32_t &devFd) override
    {
        return 0;
    }

    virtual int32_t UMountMediaFuse(int32_t userId) override
    {
        return 0;
    }

    virtual int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd) override
    {
        return 0;
    }

    virtual int32_t UMountFileMgrFuse(int32_t userId, const std::string &path) override
    {
        return 0;
    }

    virtual int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy) override
    {
        return 0;
    }

    int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb) override
    {
        return 0;
    }

    virtual int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
    {
        return 0;
    }

    virtual int32_t UMountDisShareFile(int32_t userId, const std::string &networkId)
    {
        return 0;
    }

    virtual int32_t InactiveUserPublicDirKey(uint32_t userId) override
    {
        return 0;
    }

    virtual int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback) override
    {
        return 0;
    }

    virtual int32_t UnregisterUeceActivationCallback() override
    {
        return 0;
    }

    virtual int32_t UpdateUserPublicDirPolicy(uint32_t userId) override
    {
        return 0;
    }

    virtual int32_t CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid) override
    {
        return 0;
    }
};

} // namespace StorageManager
} // namespace OHOS
#endif // I_STORAGE_MANAGER_SERVICE_H
