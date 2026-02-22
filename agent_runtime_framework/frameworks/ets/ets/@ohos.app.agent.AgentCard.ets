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

export interface AgentProvider {
  organization: string;
  url: string;
}

class AgentProviderImpl implements AgentProvider {
  public organization: string = '';
  public url: string = '';
}

export interface AgentAppInfo {
  bundleName?: string;
  moduleName?: string;
  abilityName?: string;
  deviceTypes?: string;
  minAppVersion?: string;
}

class AgentAppInfoImpl implements AgentAppInfo {
  public bundleName?: string;
  public moduleName?: string;
  public abilityName?: string;
  public deviceTypes?: string;
  public minAppVersion?: string;
}

export interface AgentCapabilities {
  streaming: boolean;
  pushNotifications?: boolean;
  stateTransitionHistory?: boolean;
  extension?: string;
  extendedAgentCard?: boolean;
}

class AgentCapabilitiesImpl implements AgentCapabilities {
  public streaming: boolean;
  public pushNotifications?: boolean;
  public stateTransitionHistory?: boolean;
  public extension?: string;
  public extendedAgentCard?: boolean;
}

export interface AgentSkill {
  id: string;
  name: string;
  description: string;
  tags: Array<string>;
  examples?: Array<string>;
  inputModes?: Array<string>;
  outputModes?: Array<string>;
  extension?: string;
}

class AgentSkillImpl implements AgentSkill {
  public id: string = '';
  public name: string = '';
  public description: string = '';
  public tags: Array<string> = {};
  public examples?: Array<string>;
  public inputModes?: Array<string>;
  public outputModes?: Array<string>;
  public extension?: string;
}

export interface AgentCard {
  agentId: string;
  appIndex: int;
  name: string;
  description: string;
  provider?: AgentProvider;
  version: string;
  documentationUrl?: string;
  capabilities: AgentCapabilities;
  defaultInputModes?: Array<string>;
  defaultOutputModes?: Array<string>;
  skills?: Array<AgentSkill>;
  extension?: string;
  category?: string;
  iconUrl?: string;
  appInfo?: AgentAppInfo;
}

class AgentCardImpl implements AgentCard {
  public agentId: string = '';
  public appIndex: int;
  public name: string = '';
  public description: string = '';
  public provider?: AgentProvider;
  public version: string = '';
  public documentationUrl?: string;
  public capabilities: AgentCapabilities = new AgentCapabilitiesImpl() as AgentCapabilities;
  public defaultInputModes?: Array<string>;
  public defaultOutputModes?: Array<string>;
  public skills?: Array<AgentSkill>;
  public extension?: string;
  public category?: string;
  public iconUrl?: string;
  public appInfo?: AgentAppInfo;
}