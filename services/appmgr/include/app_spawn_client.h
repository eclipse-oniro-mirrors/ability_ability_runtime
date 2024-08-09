/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_SPAWN_CLIENT_H
#define OHOS_ABILITY_RUNTIME_APP_SPAWN_CLIENT_H

#include <array>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

#include "appexecfwk_errors.h"
#include "appspawn.h"
#include "child_process_info.h"
#include "data_group_info.h"
#include "nocopyable.h"
#include "shared/base_shared_bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
enum class SpawnConnectionState { STATE_NOT_CONNECT, STATE_CONNECTED, STATE_CONNECT_FAILED };
using HspList = std::vector<BaseSharedBundleInfo>;
using DataGroupInfoList = std::vector<DataGroupInfo>;
const int32_t MAX_FLAG_INDEX = 32;
const int32_t MAX_PROC_NAME_LEN = 256;
const int32_t START_FLAG_BASE = 1;
struct AppSpawnStartMsg {
    int32_t uid;
    int32_t gid;
    std::vector<int32_t> gids;
    std::string procName;
    std::string soPath;
    uint32_t accessTokenId;
    std::string apl;
    std::string bundleName;
    std::string renderParam; // only nweb spawn need this param.
    int32_t pid;
    int32_t code = 0; // 0: default, MSG_APP_SPAWN; 1: MSG_SPAWN_NATIVE_PROCESS; 2: MSG_GET_RENDER_TERMINATION_STATUS
    uint32_t flags;
    int32_t bundleIndex;   // when dlp launch another app used, default is 0
    uint8_t setAllowInternet;
    uint8_t allowInternet; // hap socket allowed
    uint8_t reserved1;
    uint8_t reserved2;
    uint64_t accessTokenIdEx;
    uint32_t hapFlags = 0; // whether is pre installed hap
    HspList hspList; // list of harmony shared package
    std::string overlayInfo; // overlay hap resource path list
    DataGroupInfoList dataGroupInfoList; // list of harmony shared package
    uint32_t mountPermissionFlags;
    std::set<std::string> permissions;
    std::map<std::string, std::string> appEnv; // environment variable to be set to the process
    std::string ownerId;
    std::string provisionType;
    bool atomicServiceFlag = false;
    std::string atomicAccount = "";
    bool isolatedExtension = false; // whether is isolatedExtension
    std::string extensionSandboxPath;
    bool strictMode = false; // whether is strict mode
    std::string processType = "";
    int32_t maxChildProcess = 0;
    int32_t childProcessType = CHILD_PROCESS_TYPE_NOT_CHILD;
    std::map<std::string, int32_t> fds;
};

constexpr auto LEN_PID = sizeof(pid_t);
struct StartFlags {
    static const int COLD_START = 0;
    static const int BACKUP_EXTENSION = 1;
    static const int DLP_MANAGER = 2;
    static const int DEBUGGABLE = 3;
    static const int ASANENABLED = 4;
    static const int ACCESS_BUNDLE_DIR = 5;
    static const int NATIVEDEBUG = 6;
    static const int NO_SANDBOX = 7;
    static const int OVERLAY = 8;
    static const int BUNDLE_RESOURCES = 9;
    static const int GWP_ENABLED_FORCE = 10;
    static const int GWP_ENABLED_NORMAL = 11;
    static const int TSANENABLED = 12;
    static const int EXTENSION_CONTROLLED = 13;
};

union AppSpawnPidMsg {
    pid_t pid = 0;
    char pidBuf[LEN_PID];
};

class AppSpawnClient {
public:
    /**
     * Constructor.
     */
    explicit AppSpawnClient(bool isNWebSpawn = false);

    /**
     * Constructor by service name
     */
    explicit AppSpawnClient(const char* serviceName);

    /**
     * Destructor
     */
    virtual ~AppSpawnClient();

    /**
     * Disable copy.
     */
    DISALLOW_COPY_AND_MOVE(AppSpawnClient);

    /**
     * Try connect to appSpawn.
     */
    ErrCode OpenConnection();

    /**
     * Close the connect of appspawn.
     */
    void CloseConnection();
    
    /**
     * Return the connect state.
     */
    SpawnConnectionState QueryConnectionState() const;

    /**
     * Return the clent handle.
     */
    AppSpawnClientHandle GetAppSpawnClientHandle() const;

    /**
     * Set dac info.
     *
     * @param startMsg, request message.
     * @param reqHandle, handle for request message
     */
    int32_t SetDacInfo(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle);

    /**
     * Set mount permission.
     *
     * @param startMsg, request message.
     * @param reqHandle, handle for request message
     */
    int32_t SetMountPermission(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle);

    /**
     * Set start flags.
     *
     * @param startMsg, request message.
     * @param reqHandle, handle for request message
     */
    int32_t SetStartFlags(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle);

    /**
     * Set extra info: render-cmd, HspList, Overlay, DataGroup, AppEnv.
     *
     * @param startMsg, request message.
     * @param reqHandle, handle for request message
     */
    int32_t AppspawnSetExtMsg(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle);

    /**
     * Set extra info: provision_type, max_child_process.
     *
     * @param startMsg, request message.
     * @param reqHandle, handle for request message
     */
    int32_t AppspawnSetExtMsgMore(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle);

    /**
     * Create default appspawn msg.
     *
     * @param startMsg, request message.
     * @param reqHandle, handle for request message
     */
    int32_t AppspawnCreateDefaultMsg(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle);

    /**
     * Verify startMsg.
     *
     * @param startMsg, request message.
     */
    bool VerifyMsg(const AppSpawnStartMsg &startMsg);

    /**
     * Start request to nwebspawn process.
     */
    virtual int32_t PreStartNWebSpawnProcess();

    /**
     * AppSpawnClient core function, Start request to appSpawn.
     *
     * @param startMsg, request message.
     * @param pid, pid of app process, get from appSpawn.
     */
    virtual int32_t StartProcess(const AppSpawnStartMsg &startMsg, pid_t &pid);

    /**
     * Get render process termination status.
     *
     * @param startMsg, request message.
     * @param status, termination status of render process, get from appSpawn.
     */
    virtual int32_t GetRenderProcessTerminationStatus(const AppSpawnStartMsg &startMsg, int &status);

private:
    std::string serviceName_ = APPSPAWN_SERVER_NAME;
    AppSpawnClientHandle handle_ = nullptr;
    SpawnConnectionState state_ = SpawnConnectionState::STATE_NOT_CONNECT;

    int32_t SetChildProcessTypeStartFlag(const AppSpawnReqMsgHandle &reqHandle, int32_t childProcessType);

    int32_t SetExtMsgFds(const AppSpawnReqMsgHandle &reqHandle, const std::map<std::string, int32_t> &fds);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SPAWN_CLIENT_H
