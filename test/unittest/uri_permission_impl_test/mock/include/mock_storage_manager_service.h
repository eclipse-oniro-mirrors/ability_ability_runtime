/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

    virtual int32_t NotifyMtpMounted(const std::string &id, const std::string &path,
    const std::string &desc, const std::string &uuid) override
    {
        return E_OK;
    }

    virtual int32_t NotifyMtpUnmounted(const std::string &id, const std::string &path, const bool isBadRemove) override
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

    virtual int32_t NotifyVolumeCreated(const VolumeCore& vc) override
    {
        return E_OK;
    }

    virtual int32_t NotifyVolumeMounted(const std::string &volumeId, const std::string &fsTypeStr,
        const std::string &fsUuid, const std::string &path, const std::string &description) override
    {
        return E_OK;
    }

    virtual int32_t NotifyVolumeDamaged(const std::string &volumeId,
        const std::string &fsTypeStr, const std::string &fsUuid,
        const std::string &path, const std::string &description) override
    {
        return E_OK;
    }

    virtual int32_t NotifyVolumeStateChanged(const std::string &volumeId, uint32_t state) override
    {
        return E_OK;
    }

    virtual int32_t Mount(const std::string &volumeId) override
    {
        return E_OK;
    }

    virtual int32_t Unmount(const std::string &volumeId) override
    {
        return E_OK;
    }

    virtual int32_t TryToFix(const std::string &volumeId) override
    {
        return E_OK;
    }

    virtual int32_t GetAllVolumes(std::vector<VolumeExternal> &vecOfVol) override
    {
        return E_OK;
    }

    virtual int32_t NotifyDiskCreated(const Disk& disk) override
    {
        return E_OK;
    }

    virtual int32_t NotifyDiskDestroyed(const std::string &diskId) override
    {
        return E_OK;
    }

    virtual int32_t Partition(const std::string &diskId, int32_t type) override
    {
        return E_OK;
    }

    virtual int32_t GetAllDisks(std::vector<Disk> &vecOfDisk) override
    {
        return E_OK;
    }

    virtual int32_t GetVolumeByUuid(const std::string &fsUuid, VolumeExternal &vc) override
    {
        return E_OK;
    }

    virtual int32_t GetVolumeById(const std::string &volumeId, VolumeExternal &vc) override
    {
        return E_OK;
    }

    virtual int32_t SetVolumeDescription(const std::string &fsUuid, const std::string &description) override
    {
        return E_OK;
    }

    virtual int32_t Format(const std::string &volumeId, const std::string &fsType) override
    {
        return E_OK;
    }

    virtual int32_t GetDiskById(const std::string &diskId, Disk &disk) override
    {
        return E_OK;
    }

    virtual int32_t QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
    {
        return E_OK;
    }

    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t DeleteUserKeys(uint32_t userId) override
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

    virtual int32_t CreateShareFile(const std::vector<std::string> &uriList,
        uint32_t tokenId, uint32_t flag, std::vector<int32_t> &funcResult) override
    {
        int size = uriList.size();
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

    virtual int32_t DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &sharePathList) override
    {
        return E_OK;
    }

    virtual int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats,
        const std::string &type) override
    {
        return E_OK;
    }

    virtual int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize) override
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
};

bool StorageManagerServiceMock::isZero = true;
} // namespace StorageManager
} // namespace OHOS
#endif // MOCK_STORAGE_MANAGER_SERVICE_H
