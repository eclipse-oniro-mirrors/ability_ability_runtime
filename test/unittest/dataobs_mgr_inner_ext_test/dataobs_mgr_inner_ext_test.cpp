/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <functional>
#include <gtest/gtest.h>
#include <memory>

#include "dataobs_mgr_inner_common.h"
#include "uri.h"
#define private public
#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_inner_ext.h"
#include "dataobs_mgr_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock.h"

using namespace OHOS;
using namespace testing::ext;
using namespace testing;

using Uri = OHOS::Uri;

namespace OHOS {
namespace DataObsMgrInnerExtTest {
using namespace AAFwk;
class DataObsMgrInnerExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void RegisterObserverUtil(std::shared_ptr<DataObsMgrInnerExt> &dataObsMgrInnerExt, Uri &uri,
        const sptr<IDataAbilityObserver> &callback, uint32_t times, bool isFuzzy);

    bool UrisEqual(std::list<Uri> uri1, std::list<Uri> uri2);

    bool ChangeInfoEqual(const ChangeInfo &changeInfo1, const ChangeInfo &changeInfo2);
};

static constexpr int64_t USER_TEST = 100;

void DataObsMgrInnerExtTest::SetUpTestCase(void) {}
void DataObsMgrInnerExtTest::TearDownTestCase(void) {}
void DataObsMgrInnerExtTest::SetUp() {}
void DataObsMgrInnerExtTest::TearDown() {}

std::vector<NotifyInfo> MakeDefaultNotifyInfo(int count)
{
    std::vector<NotifyInfo> infos;
    for (int i = 0; i < count; i++) {
        NotifyInfo info("noPermission", false);
        infos.push_back(info);
    }
    return infos;
}

void DataObsMgrInnerExtTest::RegisterObserverUtil(std::shared_ptr<DataObsMgrInnerExt> &dataObsMgrInnerExt, Uri &uri,
    const sptr<IDataAbilityObserver> &callback, uint32_t times, bool isFuzzy)
{
    while (times-- > 0) {
        ObserverInfo info(0, 0, 0, USER_TEST, false);
        EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri, callback, info, isFuzzy), SUCCESS);
    }
}

bool DataObsMgrInnerExtTest::UrisEqual(std::list<Uri> uri1, std::list<Uri> uri2)
{
    if (uri1.size() != uri2.size()) {
        return false;
    }
    auto cmp = [](const Uri &first, const Uri &second) {
        return first.ToString() < second.ToString();
    };
    uri1.sort(cmp);
    uri2.sort(cmp);
    auto it1 = uri1.begin();
    auto it2 = uri2.begin();
    for (; it1 != uri1.end() && it2 != uri2.end(); it1++, it2++) {
        if (!it1->Equals(*it2)) {
            return false;
        }
    }
    return true;
}

bool DataObsMgrInnerExtTest::ChangeInfoEqual(const ChangeInfo &changeInfo1, const ChangeInfo &changeInfo2)
{
    if (changeInfo1.changeType_ != changeInfo2.changeType_) {
        return false;
    }

    if (!UrisEqual(changeInfo1.uris_, changeInfo2.uris_)) {
        return false;
    }

    if (changeInfo1.size_ != changeInfo2.size_) {
        return false;
    }

    return strncmp(reinterpret_cast<const char *>(changeInfo1.data_), reinterpret_cast<const char *>(changeInfo2.data_),
        changeInfo2.size_) == 0;
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register one non-fuzzy observer one times
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";
    Uri uri1(uriBase + "/Person");
    Uri uri2(uriBase + "/Person/2");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer, info), SUCCESS);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    ChangeInfo changeInfo = { ChangeInfo::ChangeType::OTHER, {uri1} };
    dataObsMgrInnerExt->HandleNotifyChange(changeInfo, USER_TEST, infos);
    EXPECT_TRUE(ChangeInfoEqual(observer->changeInfo_, changeInfo));

    changeInfo.uris_ = {uri1};
    observer->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange(changeInfo, USER_TEST, infos);
    EXPECT_TRUE(ChangeInfoEqual(observer->changeInfo_, changeInfo));

    changeInfo.uris_ = {uri2};
    observer->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange(changeInfo, USER_TEST, infos);
    EXPECT_TRUE(ChangeInfoEqual(observer->changeInfo_, {}));
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0200
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register one non-fuzzy observer mutiple times
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";
    Uri uri1(uriBase + "/Person");
    Uri uri2(uriBase + "/Person/2");
    Uri uri3(uriBase + "/Person/3");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());

    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer, 1, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri2, observer, DataObsMgrInnerExt::OBS_NUM_MAX - 1, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri3, observer, DataObsMgrInnerExt::OBS_NUM_MAX, false);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer->changeInfo_.uris_, { uri1 }));

    observer->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri2 } }, USER_TEST, infos);
    EXPECT_EQ(observer->onChangeCall_, DataObsMgrInnerExt::OBS_NUM_MAX - 1);

    observer->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri3 } }, USER_TEST, infos);
    EXPECT_EQ(observer->onChangeCall_, DataObsMgrInnerExt::OBS_NUM_MAX);
}

/*
* Feature: DataObsMgrInnerExt
* Function: HandleRegisterObserver test
* SubFunction: 0300
* FunctionPoints: NA
* EnvConditions: NA
* CaseDescription:Register mutiple non-fuzzy observer mutiple times
*/
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";
    Uri uri1(uriBase + "/Person");
    Uri uri2(uriBase + "/Person/2");
    Uri uri3(uriBase + "/Person/3");

    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer3(new (std::nothrow) MockDataAbilityObserverStub());
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer2, info), SUCCESS);

    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri2, observer2, info), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri2, observer3, info), SUCCESS);

    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri3, observer3, info), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri3, observer1, info), SUCCESS);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri1 }));
    EXPECT_TRUE(UrisEqual(observer2->changeInfo_.uris_, { uri1 }));
    EXPECT_TRUE(UrisEqual(observer3->changeInfo_.uris_, {}));

    observer1->ReSet();
    observer2->ReSet();
    observer3->ReSet();
    std::vector<NotifyInfo> infos2 = MakeDefaultNotifyInfo(2);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri2, uri3 } }, USER_TEST, infos2);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri3 }));
    EXPECT_TRUE(UrisEqual(observer2->changeInfo_.uris_, { uri2 }));
    EXPECT_TRUE(UrisEqual(observer3->changeInfo_.uris_, { uri2, uri3 }));
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0400
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register one fuzzy observer one times
 *          Person1 <-obs
 *           2   4
 *          3
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0400, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri123(uriBase + "/Person1/2/3");
    Uri uri14(uriBase + "/Person1/4");
    Uri uri2(uriBase + "/Person2");
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer, info, true), SUCCESS);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer->changeInfo_.uris_, { uri1 }));

    observer->ReSet();
    std::vector<NotifyInfo> infos4 = MakeDefaultNotifyInfo(4);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
            { uri12, uri123, uri14, uri2 } }, USER_TEST, infos4);
    EXPECT_TRUE(UrisEqual(observer->changeInfo_.uris_, { uri12, uri123, uri14 }));
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0500
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register one fuzzy observer mutiple times
 *          Person1 <-obs
 *           2   4 <-2*obs
 *          3     5
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0500, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri123(uriBase + "/Person1/2/3");
    Uri uri14(uriBase + "/Person1/4");
    Uri uri145(uriBase + "/Person1/4/5");

    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());

    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer, 1, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri14, observer, 2, true);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer->changeInfo_.uris_, { uri1 }));

    observer->ReSet();
    std::vector<NotifyInfo> infos4 = MakeDefaultNotifyInfo(4);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uri14, uri145, uri12, uri123 } }, USER_TEST, infos4);
    EXPECT_TRUE(UrisEqual(observer->changeInfo_.uris_, { uri14, uri14, uri14, uri145, uri145, uri145, uri12, uri123 }));
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0600
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register mutiple fuzzy observer mutiple times
 *          Person1 <-obs1
 *           2   4 <-obs2
 *    obs1->3     5
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0600, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri123(uriBase + "/Person1/2/3");
    Uri uri14(uriBase + "/Person1/4");
    Uri uri145(uriBase + "/Person1/4/5");
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri123, observer1, info, true), SUCCESS);

    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri14, observer2, info, true), SUCCESS);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri1 }));
    EXPECT_TRUE(UrisEqual(observer2->changeInfo_.uris_, {}));

    observer1->ReSet();
    observer2->ReSet();
    std::vector<NotifyInfo> infos4 = MakeDefaultNotifyInfo(4);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uri14, uri145, uri12, uri123 } }, USER_TEST, infos4);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri14, uri145, uri12, uri123, uri123 }));
    EXPECT_TRUE(UrisEqual(observer2->changeInfo_.uris_, { uri14, uri145 }));
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0700
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register mix observer mutiple times
 *          Person1 <-obs1(fuzzy)
 *         2   4 <-obs2(fuzzy and nofuzzy)
 *        3     5
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0700, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri123(uriBase + "/Person1/2/3");
    Uri uri14(uriBase + "/Person1/4");
    Uri uri145(uriBase + "/Person1/4/5");
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true), SUCCESS);

    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri14, observer2, info, true), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri14, observer2, info, false), SUCCESS);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri1 }));
    EXPECT_TRUE(UrisEqual(observer2->changeInfo_.uris_, {}));

    observer1->ReSet();
    observer2->ReSet();
    std::vector<NotifyInfo> infos4 = MakeDefaultNotifyInfo(4);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
            { uri14, uri145, uri12, uri123 } }, USER_TEST, infos4);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri14, uri145, uri12, uri123 }));
    EXPECT_TRUE(UrisEqual(observer2->changeInfo_.uris_, { uri14, uri14, uri145 }));
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0900
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register max observer
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0900, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    int res = 0;
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    int times = DataObsMgrInnerExt::OBS_NUM_MAX;
    for (int i = 0; i <= times; i++) {
        sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
        res = dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true);
        EXPECT_EQ(res, SUCCESS);
    }
    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    res = dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true);
    EXPECT_EQ(res, DATAOBS_SERVICE_OBS_LIMMIT);

    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerExt_HandleRegisterObserver_0900");
    ObserverInfo info1(1, 0, 0, USER_TEST, false);
    // other token success
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    res = dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer2, info1, true);
    EXPECT_EQ(res, SUCCESS);

    // other uri success
    Uri uri12(uriBase + "/Person1/2");
    sptr<MockDataAbilityObserverStub> observer3(new (std::nothrow) MockDataAbilityObserverStub());
    res = dataObsMgrInnerExt->HandleRegisterObserver(uri12, observer3, info, true);
    EXPECT_EQ(res, SUCCESS);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 01000
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register max observer
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_01000, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerExt_HandleRegisterObserver_01000::Start");
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    int res = 0;

    for (int token = 0; token < DataObsMgrInnerExt::OBS_NUM_MAX; token++) {
        for (int i = 0; i < DataObsMgrInnerExt::OBS_NUM_MAX; i++) {
            ObserverInfo info(token, 0, 0, USER_TEST, false);
            sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
            res = dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true);
            EXPECT_EQ(res, SUCCESS);
        }
    }
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    res = dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true);
    EXPECT_EQ(res, DATAOBS_SERVICE_OBS_LIMMIT);
    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerExt_HandleRegisterObserver_01000::Start");
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleUnregisterObserver test
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:UnRegister observer
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleUnregisterObserver_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase1 = "datashare://Authority1/com.domainname.dataability.persondata";
    std::string uriBase2 = "datashare://Authority2/com.domainname.dataability.persondata";

    Uri uri1(uriBase1 + "/Person1");
    Uri uri12(uriBase1 + "/Person1/2");
    Uri uri2(uriBase2 + "/Person2");

    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true), SUCCESS);
    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_TRUE(UrisEqual(observer1->changeInfo_.uris_, { uri1 }));
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 1);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri1, observer1), SUCCESS);
    observer1->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_EQ(observer1->onChangeCall_, 0);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleUnregisterObserver test
 * SubFunction: 0200
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:UnRegister one observers mutiple times
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleUnregisterObserver_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");

    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 0);

    ObserverInfo info(0, 0, 0, USER_TEST, false);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer, info, true), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 1);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.find(observer)->second->ref, 2);

    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer, info, false), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 1);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.find(observer)->second->ref, 3);
    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri1, observer), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 0);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri1, observer), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 0);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1 } }, USER_TEST, infos);
    EXPECT_EQ(observer->onChangeCall_, 2);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleUnregisterObserver test
 * SubFunction: 0300
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:UnRegister mutiple observers on mutiple uri
 *          Person1
 *           2   3<-2*obs1、obs2
 *        obs1->4 5<-obs2
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleUnregisterObserver_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri13(uriBase + "/Person1/3");
    Uri uri134(uriBase + "/Person1/3/4");
    Uri uri135(uriBase + "/Person1/3/5");
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri13, observer, info, true), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri13, observer, info, false), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri134, observer, info, false), SUCCESS);

    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri13, observer2, info, true), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri135, observer2, info, true), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 2);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.find(observer2)->second->ref, 3);

    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri135, observer), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri135, observer2), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 2);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.find(observer2)->second->ref, 2);

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri135 } }, USER_TEST, infos);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri13, observer2), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri135 } }, USER_TEST, infos);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 1);

    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri13, observer), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 1);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri13 } }, USER_TEST, infos);

    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri134, observer), SUCCESS);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 0);
    EXPECT_TRUE(dataObsMgrInnerExt->root_->childrens_.empty());
    EXPECT_EQ(observer->onChangeCall_, 2);
    EXPECT_EQ(observer2->onChangeCall_, 1);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: Register and UnRegister test
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register and UnRegister when observers over limmit
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_RegisterAndUnRegister_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    Uri uri1(uriBase + "/Person1");
    Uri uri2(uriBase + "/Person2");

    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer1, AAFwk::DataObsMgrInnerExt::OBS_NUM_MAX + 1, false);
    auto obsRecipientRef1 = dataObsMgrInnerExt->obsRecipientRefs.find(observer1);
    EXPECT_TRUE(obsRecipientRef1 != dataObsMgrInnerExt->obsRecipientRefs.end() && obsRecipientRef1->second->ref == 52);
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.find(observer2) == dataObsMgrInnerExt->obsRecipientRefs.end());
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, false),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer2, info, true),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(obsRecipientRef1->second->ref, 52);
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.find(observer2) == dataObsMgrInnerExt->obsRecipientRefs.end());

    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(2);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri1, observer1), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri2, observer1), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri2, observer2), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos);
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.find(observer1) == dataObsMgrInnerExt->obsRecipientRefs.end());
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.find(observer2) == dataObsMgrInnerExt->obsRecipientRefs.end());
    EXPECT_EQ(observer1->onChangeCall_, 51);
    EXPECT_EQ(observer2->onChangeCall_, 0);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: Register and UnRegister test
 * SubFunction: 0200
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register and UnRegister when observers over limmit
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_RegisterAndUnRegister_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    Uri uri1(uriBase + "/Person1");
    Uri uri2(uriBase + "/Person2");

    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri2, observer1, 20, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri2, observer2, 31, false);
    auto obsRecipientRef1 = dataObsMgrInnerExt->obsRecipientRefs.find(observer1);
    auto obsRecipientRef2 = dataObsMgrInnerExt->obsRecipientRefs.find(observer2);
    EXPECT_TRUE(obsRecipientRef1 != dataObsMgrInnerExt->obsRecipientRefs.end() && obsRecipientRef1->second->ref == 21);
    EXPECT_TRUE(obsRecipientRef2 != dataObsMgrInnerExt->obsRecipientRefs.end() && obsRecipientRef2->second->ref == 32);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri2, observer1, info, false),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri2, observer2, info, false),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(obsRecipientRef1->second->ref, 21);
    EXPECT_EQ(obsRecipientRef2->second->ref, 32);

    std::vector<NotifyInfo> infos2 = MakeDefaultNotifyInfo(2);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos2);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri1, observer1), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos2);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri2, observer1), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos2);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(uri2, observer2), SUCCESS);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri1, uri2 } }, USER_TEST, infos2);
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.find(observer1) == dataObsMgrInnerExt->obsRecipientRefs.end());
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.find(observer2) == dataObsMgrInnerExt->obsRecipientRefs.end());
    EXPECT_EQ(observer1->onChangeCall_, 40);
    EXPECT_EQ(observer2->onChangeCall_, 93);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: Register and UnRegister test
 * SubFunction: 0300
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Register and UnRegister when uri path size over limit
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_RegisterAndUnRegister_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerExt_RegisterAndUnRegister_0300::Start");
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority/com.domainname.dataability.persondata";
    // make a long uri path, which is over limit MAX_URI_PATH_SIZE
    for (uint32_t i = 0; i < DataObsMgrInnerExt::MAX_URI_PATH_SIZE; i++) {
        uriBase += "/test";
    }
    Uri uri1(uriBase);
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());

    int res = dataObsMgrInnerExt->HandleRegisterObserver(uri1, observer1, info, true);
    EXPECT_EQ(res, DATAOBS_INVALID_URI);

    res = dataObsMgrInnerExt->HandleUnregisterObserver(uri1, observer1);
    EXPECT_EQ(res, DATAOBS_INVALID_URI);
    TAG_LOGE(AAFwkTag::DBOBSMGR, "DataObsMgrInnerExt_RegisterAndUnRegister_0300::Start");
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleUnregisterObserver no uri
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:HandleUnregisterObserver one observers on all uri
 *          Person1<-5*obs1(fuzzy)+25*obs2
 *   10、20->2   3<-15*obs1+15*obs2
 *     20*obs1->4 5<-25*obs1+5*obs2
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleUnregisterObserverAll_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";

    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri13(uriBase + "/Person1/3");
    Uri uri134(uriBase + "/Person1/3/4");
    Uri uri135(uriBase + "/Person1/3/5");

    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());

    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer1, 5, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri12, observer1, 10, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri13, observer1, 15, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri134, observer1, 20, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri135, observer1, 25, false);

    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer2, 25, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri12, observer2, 20, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri13, observer2, 15, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri134, observer2, 10, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri135, observer2, 5, false);

    std::vector<NotifyInfo> infos5 = MakeDefaultNotifyInfo(5);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uri1, uri12, uri13, uri134, uri135 } }, USER_TEST, infos5);
    EXPECT_EQ(observer1->onChangeCall_, 95);
    EXPECT_EQ(observer2->onChangeCall_, 205);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(observer1), SUCCESS);
    observer1->ReSet();
    observer2->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uri1, uri12, uri13, uri134, uri135 } }, USER_TEST, infos5);
    EXPECT_EQ(observer1->onChangeCall_, 0);
    EXPECT_EQ(observer2->onChangeCall_, 205);
    EXPECT_EQ(dataObsMgrInnerExt->HandleUnregisterObserver(observer2), SUCCESS);
    EXPECT_TRUE(dataObsMgrInnerExt->root_->childrens_.empty());
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: DeathRecipient test
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:DeathRecipient
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_DeathRecipient_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    Uri uri1(uriBase + "/Person1");
    Uri uri12(uriBase + "/Person1/2");
    Uri uri13(uriBase + "/Person1/3");
    Uri uri134(uriBase + "/Person1/3/4");
    Uri uri135(uriBase + "/Person1/3/5");

    sptr<MockDataAbilityObserverStub> observer1(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer2(new (std::nothrow) MockDataAbilityObserverStub());

    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer1, 5, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri12, observer1, 10, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri13, observer1, 15, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri134, observer1, 20, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri135, observer1, 25, false);

    RegisterObserverUtil(dataObsMgrInnerExt, uri1, observer2, 25, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri12, observer2, 20, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri13, observer2, 15, true);
    RegisterObserverUtil(dataObsMgrInnerExt, uri134, observer2, 10, false);
    RegisterObserverUtil(dataObsMgrInnerExt, uri135, observer2, 5, false);

    std::vector<NotifyInfo> infos5 = MakeDefaultNotifyInfo(5);
    EXPECT_EQ(dataObsMgrInnerExt->obsRecipientRefs.size(), 2);
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uri1, uri12, uri13, uri134, uri135 } }, USER_TEST, infos5);
    EXPECT_EQ(observer1->onChangeCall_, 95);
    EXPECT_EQ(observer2->onChangeCall_, 205);

    auto it = dataObsMgrInnerExt->obsRecipientRefs.find(observer1->AsObject());
    EXPECT_TRUE(it != dataObsMgrInnerExt->obsRecipientRefs.end() && it->second->deathRecipient != nullptr);
    it->second->deathRecipient->OnRemoteDied(observer1->AsObject());

    observer1->ReSet();
    observer2->ReSet();
    dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uri1, uri12, uri13, uri134, uri135 } }, USER_TEST, infos5);
    EXPECT_EQ(observer1->onChangeCall_, 0);
    EXPECT_EQ(observer2->onChangeCall_, 205);
    dataObsMgrInnerExt->OnCallBackDied(observer2->AsObject());
    EXPECT_TRUE(dataObsMgrInnerExt->root_->childrens_.empty());
    EXPECT_TRUE(dataObsMgrInnerExt->obsRecipientRefs.empty());
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: DeathRecipient test
 * SubFunction: 0200
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:DeathRecipient when dataObsMgrInnerExt release
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_DeathRecipient_0200, TestSize.Level1)
{
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    Uri uri(uriBase + "/Person1");

    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    {
        ObserverInfo info(0, 0, 0, USER_TEST, false);
        std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
        EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri, observer, info, false), SUCCESS);

        auto it = dataObsMgrInnerExt->obsRecipientRefs.find(observer->AsObject());
        EXPECT_TRUE(it != dataObsMgrInnerExt->obsRecipientRefs.end() && it->second->deathRecipient != nullptr);
        deathRecipient = it->second->deathRecipient;
    }

    deathRecipient->OnRemoteDied(observer->AsObject());
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: AddObsDeathRecipient test
 * SubFunction: 0800
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Add obs death recipient over max
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_AddObsDeathRecipientOverMax_0800, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    Uri uri(uriBase + "/Person1");

    sptr<MockDataAbilityObserverStub> observer = (new (std::nothrow) MockDataAbilityObserverStub());
    auto deathRecipientRef = std::make_shared<DataObsMgrInnerExt::DeathRecipientRef>(nullptr);
    deathRecipientRef->ref = std::numeric_limits<uint32_t>::max() - 1;

    dataObsMgrInnerExt->obsRecipientRefs.emplace(observer, deathRecipientRef);

    EXPECT_TRUE(dataObsMgrInnerExt->AddObsDeathRecipient(observer));
    deathRecipientRef->ref++;
    EXPECT_FALSE(dataObsMgrInnerExt->AddObsDeathRecipient(observer));
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri, observer, info), DATAOBS_SERVICE_OBS_LIMMIT);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleRegisterObserver test
 * SubFunction: 0800
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:HandleRegisterObserver muti threads
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleRegisterObserver_0800, TestSize.Level1)
{
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    std::vector<Uri> uris;
    uris.emplace_back(uriBase + "/1");
    uris.emplace_back(uriBase + "/2");
    uris.emplace_back(uriBase + "/1/3");
    uris.emplace_back(uriBase + "/2/4");
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();

    auto func = [](std::vector<Uri> &uris, std::shared_ptr<DataObsMgrInnerExt> obsMgr,
                    sptr<MockDataAbilityObserverStub> &obs) {
        for (uint32_t i = 0; i < uris.size() * 5; ++i) {
            ObserverInfo info(0, 0, 0, USER_TEST, false);
            EXPECT_EQ(obsMgr->HandleRegisterObserver(uris[i % uris.size()], obs, info, false), SUCCESS);
        }
        obs->Notify();
    };

    sptr<MockDataAbilityObserverStub> observer1 = (new (std::nothrow) MockDataAbilityObserverStub());
    std::thread thread1(std::bind(func, uris, dataObsMgrInnerExt, observer1));
    thread1.detach();

    sptr<MockDataAbilityObserverStub> observer2 = (new (std::nothrow) MockDataAbilityObserverStub());
    std::thread thread2(std::bind(func, uris, dataObsMgrInnerExt, observer2));
    thread2.detach();

    observer1->Wait();
    observer2->Wait();

    std::vector<NotifyInfo> infos2 = MakeDefaultNotifyInfo(2);
    EXPECT_EQ(dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uris[0], uris[1] } }, USER_TEST, infos2), SUCCESS);
    EXPECT_EQ(observer1->onChangeCall_, 10);
    EXPECT_EQ(observer2->onChangeCall_, 10);
    dataObsMgrInnerExt->HandleUnregisterObserver(observer1);

    observer1->ReSet();
    observer2->ReSet();
    EXPECT_EQ(dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT,
        { uris[2], uris[3] } }, USER_TEST, infos2), SUCCESS);
    EXPECT_EQ(observer1->onChangeCall_, 0);
    EXPECT_EQ(observer2->onChangeCall_, 10);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: HandleNotifyChange test
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:RegisterObserver when NotifyChange
 */
HWTEST_F(DataObsMgrInnerExtTest, DataObsMgrInnerExt_HandleNotifyChange_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInnerExt> dataObsMgrInnerExt = std::make_shared<DataObsMgrInnerExt>();
    std::string uriBase = "datashare://Authority1/com.domainname.dataability.persondata";
    Uri uri(uriBase + "/Person");
    auto sch = uri.GetScheme();

    sptr<MockDataAbilityObserverStub> observer1 = (new (std::nothrow) MockDataAbilityObserverStub());
    sptr<MockDataAbilityObserverStub> observer2 = (new (std::nothrow) MockDataAbilityObserverStub());
    ObserverInfo info(0, 0, 0, USER_TEST, false);
    dataObsMgrInnerExt->HandleRegisterObserver(uri, observer1, info);

    observer1->func = [&dataObsMgrInnerExt, &observer2, &uri]() {
        ObserverInfo info(0, 0, 0, USER_TEST, false);
        EXPECT_EQ(dataObsMgrInnerExt->HandleRegisterObserver(uri, observer2, info), SUCCESS);
    };
    std::vector<NotifyInfo> infos = MakeDefaultNotifyInfo(1);
    EXPECT_EQ(dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri } },
        USER_TEST, infos), SUCCESS);
    EXPECT_EQ(observer1->onChangeCall_, 1);
    observer1->func = nullptr;
    EXPECT_EQ(dataObsMgrInnerExt->HandleNotifyChange({ ChangeInfo::ChangeType::INSERT, { uri } },
        USER_TEST, infos), SUCCESS);
    EXPECT_EQ(observer2->onChangeCall_, 1);
}

} // namespace DataObsMgrInnerExtTest
} // namespace OHOS
