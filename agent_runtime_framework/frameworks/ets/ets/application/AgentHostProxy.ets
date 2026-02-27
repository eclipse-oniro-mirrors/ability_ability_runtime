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

/**
 * The proxy object passed to agent extension when connected.
 * Used by agent extension to send callbacks to the host application.
 */
export default interface AgentHostProxy {
  /**
   * Send data to host application.
   *
   * @param data - Indicates the data to send.
   */
  sendData(data: string): void;

  /**
   * Send authorization to host application.
   *
   * @param data - Indicates the data to send.
   */
  authorize(data: string): void;
}

export class AgentHostProxyImpl implements AgentHostProxy {
  nativeConnectorProxy: long = 0;
  native nativeSendData(data: string): void;
  native nativeAuthorize(data: string): void;

  sendData(data: string): void {
    this.nativeSendData(data);
  }

  authorize(data: string): void {
    this.nativeAuthorize(data);
  }
}
