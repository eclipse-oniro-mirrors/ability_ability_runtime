/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef MOCK_STORAGE_MANAGER_SERVICE_H
#define MOCK_STORAGE_MANAGER_SERVICE_H

#include "gmock/gmock.h"
#include "iremote_stub.h"
#include "istorage_manager.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerServiceMock :  public IRemoteStub<IStorageManager> {
public:
    static bool isZero;
    int code_ = 0;
    int E_OK = 0;
    StorageManagerServiceMock() : code_(0) {}
    virtual ~StorageManagerServiceMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    int32_t InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return E_OK;
    }

    virtual int32_t PrepareAddUser(int32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t RemoveUser(int32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t PrepareStartUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t StopUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t CompleteAddUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize) override
    {
        return E_OK;
    }

    virtual int32_t GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize) override
    {
        return E_OK;
    }

    virtual int32_t SetDirEncryptionPolicy(uint32_t userId,
        const std::string &dirPath, uint32_t level) override
    {
        return E_OK;
    }

    virtual int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats,
        int32_t index, uint32_t statFlag) override
    {
        return E_OK;
    }

    virtual int32_t GetSystemSize(int64_t &systemSize) override
    {
        return E_OK;
    }

    virtual int32_t GetTotalSize(int64_t &totalSize) override
    {
        return E_OK;
    }

    virtual int32_t GetFreeSize(int64_t &freeSize) override
    {
        return E_OK;
    }

    virtual int32_t GetTotalInodes(int64_t &totalInodes) override
    {
        return E_OK;
    }

    virtual int32_t GetFreeInodes(int64_t &freeInodes) override
    {
        return E_OK;
    }

    virtual int32_t GetCurrentBundleInodes(int64_t &curInodes) override
    {
        return E_OK;
    }

    virtual int32_t GetSystemDataSize(int64_t &otherUidSizeSum) override
    {
        return E_OK;
    }

    virtual int32_t GetUserStorageStats(StorageStats &storageStats) override
    {
        return E_OK;
    }

    virtual int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats) override
    {
        return E_OK;
    }

    virtual int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag) override
    {
        return E_OK;
    }

    virtual int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) override
    {
        return E_OK;
    }

    virtual int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                 const std::vector<uint8_t> &newSecret,
                                                 uint64_t secureUid,
                                                 uint32_t userId,
                                                 const std::vector<std::vector<uint8_t>> &plainText) override
    {
        return E_OK;
    }

    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) override
    {
        return E_OK;
    }

    virtual int32_t InactiveUserKey(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t LockUserScreen(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t UnlockUserScreen(uint32_t userId,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret) override
    {
        return E_OK;
    }

    virtual int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus) override
    {
        return E_OK;
    }

    virtual int32_t GenerateAppkey(uint32_t hashId, uint32_t userId,
                                   std::string &keyId, bool needReset = false) override
    {
        return E_OK;
    }

    virtual int32_t DeleteAppkey(const std::string &keyId) override
    {
        return E_OK;
    }

    virtual int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount) override
    {
        return E_OK;
    }

    virtual int32_t CreateRecoverKey(uint32_t userId,
                                     uint32_t userType,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret) override
    {
        return E_OK;
    }

    virtual int32_t SetRecoverKey(const std::vector<uint8_t> &key) override
    {
        return E_OK;
    }

    virtual int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType,
                                               const std::vector<uint8_t> &key) override
    {
        return E_OK;
    }

    virtual int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false) override
    {
        return E_OK;
    }

    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive) override
    {
        return E_OK;
    }

    virtual int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override
    {
        return E_OK;
    }

    virtual int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override
    {
        return E_OK;
    }

    virtual int32_t CreateShareFile(const StorageFileRawData &storageShareRawData,
        uint32_t tokenId, uint32_t flag, std::vector<int32_t> &funcResult) override
    {
        int32_t size = static_cast<int32_t>(storageShareRawData.size);
        if (size <= 0) {
            return -1;
        }
        if (isZero) {
            funcResult.assign(size, ERR_OK);
            return ERR_OK;
        }
        funcResult.assign(size, -1);
        return E_OK;
    }

    virtual int32_t DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData) override
    {
        return E_OK;
    }

    virtual int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats,
        const std::string &type) override
    {
        return E_OK;
    }

    virtual int32_t MountMediaFuse(int32_t userId, int32_t &devFd) override
    {
        return E_OK;
    }

    virtual int32_t UMountMediaFuse(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd) override
    {
        return E_OK;
    }

    virtual int32_t UMountFileMgrFuse(int32_t userId, const std::string &path) override
    {
        return E_OK;
    }

    virtual int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy) override
    {
        return E_OK;
    }

    int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb) override
    {
        return E_OK;
    }

    virtual int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
    {
        return E_OK;
    }

    virtual int32_t UMountDisShareFile(int32_t userId, const std::string &networkId)
    {
        return E_OK;
    }

    virtual int32_t UMountDisShareFile(const std::vector<std::string> &distributeDirs)
    {
        return E_OK;
    }

    virtual int32_t InactiveUserPublicDirKey(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t UpdateUserPublicDirPolicy(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback) override
    {
        return E_OK;
    }

    virtual int32_t UnregisterUeceActivationCallback() override
    {
        return E_OK;
    }

    virtual int32_t CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid) override
    {
        return E_OK;
    }

    virtual int32_t NotifyUserChangedEvent(uint32_t userId, uint32_t eventType) override
    {
        return E_OK;
    }

    virtual int32_t EraseAllUserEncryptedKeys() override
    {
        return E_OK;
    }

    virtual int32_t SetExtBundleStats(uint32_t userId, const ExtBundleStats &stats) override
    {
        return E_OK;
    }

    virtual int32_t GetExtBundleStats(uint32_t userId, ExtBundleStats &stats) override
    {
        return E_OK;
    }

    virtual int32_t GetAllExtBundleStats(uint32_t userId, std::vector<ExtBundleStats> &statsVec) override
    {
        return E_OK;
    }

    virtual int32_t ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs) override
    {
        return E_OK;
    }

    virtual int32_t NotifyCreateBundleDataDirWithEl(uint32_t userId, uint8_t elx) override
    {
        return E_OK;
    }

    virtual int32_t QueryActiveOsAccountIds(std::vector<int32_t> &ids) override
    {
        return E_OK;
    }

    virtual int32_t IsOsAccountExists(unsigned int userId, bool &isOsAccountExists) override
    {
        return E_OK;
    }

    virtual int32_t MountDlpFuse(const std::string &dstPath, int32_t &fuseFd) override
    {
        return E_OK;
    }

    virtual int32_t UMountDlpFuse(const std::string &dstPath) override
    {
        return E_OK;
    }
};

bool StorageManagerServiceMock::isZero = true;
} // namespace StorageManager
} // namespace OHOS
#endif // MOCK_STORAGE_MANAGER_SERVICE_H
