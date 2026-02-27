/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
import Want from '@ohos.app.ability.Want';
import { AgentCard } from 'application.AgentCard';
import { AgentProxy } from 'application.AgentProxy';
import { AgentExtensionConnectCallback } from 'application.AgentExtensionConnectCallback';
import { AsyncCallbackWrapper } from './utils/AgentUtils';
import { BusinessError } from '@ohos.base';

export default namespace agentManager {
  loadLibraryWithPermissionCheck("agentmanager_ani.z", "@ohos.app.agent.agentManager")

  export native function nativeGetAllAgentCards(asyncCallback: AsyncCallbackWrapper<Array<AgentCard>>): void;
  export native function nativeGetAgentCardsByBundleName(bundleName: string, asyncCallback: AsyncCallbackWrapper<Array<AgentCard>>): void;
  export native function nativeGetAgentCardByAgentId(bundleName: string, agentId: string, asyncCallback: AsyncCallbackWrapper<AgentCard>): void;
  export native function nativeConnectAgentExtensionAbility(want: Want, agentId: string,
    connectCallback: AgentExtensionConnectCallback, asyncCallback: AsyncCallbackWrapper<AgentProxy>): void;
  export native function nativeDisconnectAgentExtensionAbility(proxy: AgentProxy,
    asyncCallback: AsyncCallbackWrapper<void>): void;

  export function getAllAgentCards(): Promise<Array<AgentCard>> {
    let p: Promise<Array<AgentCard>> = new Promise<Array<AgentCard>>(
      (res: (data: Array<AgentCard>) => void, rej: (error: BusinessError) => void): void => {
      let asyncCallbackWrapper = new AsyncCallbackWrapper<Array<AgentCard>>(
        (err: BusinessError | null, data: Array<AgentCard> | undefined) => {
          if (err == null || err.code == 0) {
            res(data as Array<AgentCard>);
          } else {
            rej(err);
          }
        });
      taskpool.execute((): void => {
        agentManager.nativeGetAllAgentCards(asyncCallbackWrapper);
      });
    });
    return p;
  }

  export function getAgentCardsByBundleName(bundleName: string): Promise<Array<AgentCard>> {
    let p: Promise<Array<AgentCard>> = new Promise<Array<AgentCard>>(
      (res: (data: Array<AgentCard>) => void, rej: (error: BusinessError) => void): void => {
      let asyncCallbackWrapper = new AsyncCallbackWrapper<Array<AgentCard>>(
        (err: BusinessError | null, data: Array<AgentCard> | undefined) => {
          if (err == null || err.code == 0) {
            res(data as Array<AgentCard>);
          } else {
            rej(err);
          }
        });
      taskpool.execute((): void => {
        agentManager.nativeGetAgentCardsByBundleName(bundleName, asyncCallbackWrapper);
      });
    });
    return p;
  }

  export function getAgentCardByAgentId(bundleName: string, agentId: string): Promise<AgentCard> {
    let p: Promise<AgentCard> = new Promise<AgentCard>(
      (res: (data: AgentCard) => void, rej: (error: BusinessError) => void): void => {
      let asyncCallbackWrapper = new AsyncCallbackWrapper<AgentCard>(
        (err: BusinessError | null, data: AgentCard | undefined) => {
          if (err == null || err.code == 0) {
            res(data as AgentCard);
          } else {
            rej(err);
          }
        });
      taskpool.execute((): void => {
        agentManager.nativeGetAgentCardByAgentId(bundleName, agentId, asyncCallbackWrapper);
      });
    });
    return p;
  }

  export function connectAgentExtensionAbility(want: Want, agentId: string,
    connectCallback: AgentExtensionConnectCallback): Promise<AgentProxy> {
    let p: Promise<AgentProxy> = new Promise<AgentProxy>(
      (res: (data: AgentProxy) => void, rej: (error: BusinessError) => void): void => {
      let asyncCallbackWrapper = new AsyncCallbackWrapper<AgentProxy>(
        (err: BusinessError | null, data: AgentProxy | undefined) => {
          if (err == null || err.code == 0) {
            res(data as AgentProxy);
          } else {
            rej(err);
          }
        });
      taskpool.execute((): void => {
        agentManager.nativeConnectAgentExtensionAbility(want, agentId, connectCallback, asyncCallbackWrapper);
      });
    });
    return p;
  }

  export function disconnectAgentExtensionAbility(proxy: AgentProxy): Promise<void> {
    let p: Promise<void> = new Promise<void>(
      (res: (data: undefined) => void, rej: (error: BusinessError) => void): void => {
      let asyncCallbackWrapper = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          res(undefined);
        } else {
          rej(err);
        }
      });
      taskpool.execute((): void => {
        agentManager.nativeDisconnectAgentExtensionAbility(proxy, asyncCallbackWrapper);
      });
    });
    return p;
  }
} // namespace agentManager
