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

#include <string>

#include <gtest/gtest.h>

#include "agent_manager_caller_identity.h"
#include "ipc_skeleton.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;

namespace {
// Captures a real calling-identity token from the IPC skeleton. SetCallingIdentity
// rejects strings not in the skeleton's internal format, so tests must use real tokens.
std::string CaptureRealIdentity()
{
    std::string id = IPCSkeleton::ResetCallingIdentity();
    IPCSkeleton::SetCallingIdentity(id);
    return id;
}

void RestoreIdentity(const std::string &id)
{
    std::string mutableId = id;
    IPCSkeleton::SetCallingIdentity(mutableId);
}
}  // namespace

class AgentManagerCallerIdentityTest : public testing::Test {
public:
    void SetUp() override
    {
        baseIdentity_ = CaptureRealIdentity();
    }
    void TearDown() override
    {
        RestoreIdentity(baseIdentity_);
    }

private:
    std::string baseIdentity_;
};

/**
 * @tc.name      ScopeCapturesOriginalIdentity
 * @tc.desc      Constructing AgentManagerCallerIdentityScope captures the active IPC identity
 *               and exposes it via GetOriginalIdentity().
 */
HWTEST_F(AgentManagerCallerIdentityTest, ScopeCapturesOriginalIdentity, TestSize.Level1)
{
    const std::string before = CaptureRealIdentity();
    {
        AgentManagerCallerIdentityScope scope;
        const std::string &captured = scope.GetOriginalIdentity();
        EXPECT_FALSE(captured.empty());
        EXPECT_EQ(captured, before);
    }
}

/**
 * @tc.name      ScopeRestoresIdentityOnDestruction
 * @tc.desc      When the scope exits, the IPC identity that was active at construction
 *               must be restored even if inner code mutated it.
 */
HWTEST_F(AgentManagerCallerIdentityTest, ScopeRestoresIdentityOnDestruction, TestSize.Level1)
{
    // Use real identity tokens so SetCallingIdentity accepts the format.
    const std::string innerIdentity = CaptureRealIdentity();
    const std::string outerIdentity = CaptureRealIdentity();
    RestoreIdentity(outerIdentity);
    {
        AgentManagerCallerIdentityScope scope;
        // Simulate inner code switching to a different valid identity.
        RestoreIdentity(innerIdentity);
        std::string inside = IPCSkeleton::ResetCallingIdentity();
        RestoreIdentity(inside);
        EXPECT_EQ(inside, innerIdentity);
    }
    // After the scope exits, the identity captured at construction (outerIdentity) is restored.
    std::string after = IPCSkeleton::ResetCallingIdentity();
    RestoreIdentity(after);
    EXPECT_EQ(after, outerIdentity);
}

/**
 * @tc.name      NestedScopesRestoreInLifoOrder
 * @tc.desc      Nested scopes must restore identities in LIFO order.
 */
HWTEST_F(AgentManagerCallerIdentityTest, NestedScopesRestoreInLifoOrder, TestSize.Level1)
{
    const std::string outer = CaptureRealIdentity();
    const std::string inner = CaptureRealIdentity();
    RestoreIdentity(outer);
    {
        AgentManagerCallerIdentityScope outerScope;
        RestoreIdentity(inner);
        {
            AgentManagerCallerIdentityScope innerScope;
            EXPECT_EQ(innerScope.GetOriginalIdentity(), inner);
        }
        // After the inner scope exits, the inner identity is restored.
        std::string afterInner = IPCSkeleton::ResetCallingIdentity();
        RestoreIdentity(afterInner);
        EXPECT_EQ(afterInner, inner);
    }
    // After the outer scope exits, the original identity is restored.
    std::string afterOuter = IPCSkeleton::ResetCallingIdentity();
    RestoreIdentity(afterOuter);
    EXPECT_EQ(afterOuter, outer);
}
