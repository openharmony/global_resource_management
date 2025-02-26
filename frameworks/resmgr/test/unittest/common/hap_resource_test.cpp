/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hap_resource_test.h"

#include <climits>
#include <gtest/gtest.h>

#include "hap_parser.h"
#include "hap_resource.h"
#include "test_common.h"
#include "utils/date_utils.h"
#include "utils/errors.h"
#include "utils/string_utils.h"

#define private public

using namespace OHOS::Global::Resource;
using namespace testing::ext;
namespace {
class HapResourceTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HapResourceTest::SetUpTestCase(void)
{
    // step 1: input testsuit setup step
    g_logLevel = LOG_DEBUG;
}

void HapResourceTest::TearDownTestCase(void)
{
    // step 2: input testsuit teardown step
}

void HapResourceTest::SetUp()
{
    // step 3: input testcase setup step
}

void HapResourceTest::TearDown()
{
    // step 4: input testcase teardown step
}

//void TestGetIdValuesByName(const HapResource *pResource)
void TestGetIdValuesByName(const std::shared_ptr<HapResource> pResource)
{
    std::string name = std::string("app_name");
    auto start = CurrentTimeUsec();
    auto idValues = pResource->GetIdValuesByName(name, ResType::STRING);
    auto cost = CurrentTimeUsec() - start;
    // 3 compare with the size of LimitPaths
    EXPECT_EQ(static_cast<size_t>(3), idValues->GetLimitPathsConst().size());
    RESMGR_HILOGD(RESMGR_TAG, "GetIdValues by name cost: %ld us.", cost);
    PrintIdValues(idValues);
    {
        auto limitPath = idValues->GetLimitPathsConst()[0];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "base");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "About");
    }
    {
        auto limitPath = idValues->GetLimitPathsConst()[1];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "en_Latn_US");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "App Name");
    }
    {
        auto limitPath = idValues->GetLimitPathsConst()[2];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "zh_Hans_CN");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "应用名称");
    }
}

/*
 * this test shows how to load a hap, defaultConfig set to null
 * @tc.name: HapResourceFuncTest001
 * @tc.desc: Test Load & GetIdValues & GetIdValuesByName function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(HapResourceTest, HapResourceFuncTest001, TestSize.Level0)
{
    auto start = CurrentTimeUsec();
    std::shared_ptr<ResConfigImpl> rc = nullptr;
    auto pResource = HapResource::LoadFromIndex(FormatFullPath(g_resFilePath).c_str(), rc);
    auto cost = CurrentTimeUsec() - start;
    RESMGR_HILOGD(RESMGR_TAG, "load cost: %ld us.", cost);

    if (pResource == nullptr) {
        EXPECT_TRUE(false);
        return;
    }

    int id = pResource->GetIdByName("app_name", ResType::STRING);
    start = CurrentTimeUsec();
    auto idValues = pResource->GetIdValues(id);
    cost = CurrentTimeUsec() - start;
    EXPECT_EQ(static_cast<size_t>(3), idValues->GetLimitPathsConst().size());
    RESMGR_HILOGD(RESMGR_TAG, "GetIdValues by id cost: %ld us.", cost);
    PrintIdValues(idValues);
    {
        auto limitPath = idValues->GetLimitPathsConst()[0];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "base");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "About");
    }
    {
        auto limitPath = idValues->GetLimitPathsConst()[1];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "en_Latn_US");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "App Name");
    }
    {
        auto limitPath = idValues->GetLimitPathsConst()[2];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "zh_Hans_CN");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "应用名称");
    }
    TestGetIdValuesByName(pResource);
}

//void GetIdValuesByNameFuncTest002(const HapResource *pResource, int id)
void GetIdValuesByNameFuncTest002(const std::shared_ptr<HapResource> pResource, int id)
{
    std::string name = std::string("app_name");
    auto start = CurrentTimeUsec();
    auto idValues = pResource->GetIdValuesByName(name, ResType::STRING);
    auto cost = CurrentTimeUsec() - start;
    EXPECT_EQ(static_cast<size_t>(3), idValues->GetLimitPathsConst().size()); // 3 means the number of candidates
    RESMGR_HILOGD(RESMGR_TAG, "GetIdValues by name cost: %ld us.", cost);
    PrintIdValues(idValues);

    {
        auto limitPath = idValues->GetLimitPathsConst()[0];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "base");
        EXPECT_TRUE(limitPath->GetIdItem()->id_ == static_cast<uint32_t>(id));
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "About");
    }
    {
        auto limitPath = idValues->GetLimitPathsConst()[1];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "en_Latn_US");
        EXPECT_TRUE(limitPath->GetIdItem()->id_ == static_cast<uint32_t>(id));
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "App Name");
    }
}

/*
 * load a hap, set config en_US
 * @tc.name: HapResourceFuncTest002
 * @tc.desc: Test Load & GetIdValues & GetIdValuesByName function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(HapResourceTest, HapResourceFuncTest002, TestSize.Level1)
{
    auto rc = std::make_shared<ResConfigImpl>();
    rc->SetLocaleInfo("en", nullptr, "US");
    std::string resPath = FormatFullPath(g_resFilePath);
    const char *path = resPath.c_str();

    auto start = CurrentTimeUsec();
    auto pResource = HapResource::LoadFromIndex(path, rc);
    auto cost = CurrentTimeUsec() - start;
    RESMGR_HILOGD(RESMGR_TAG, "load cost: %ld us.", cost);

    if (pResource == nullptr) {
        EXPECT_TRUE(false);
        return;
    }

    int id = pResource->GetIdByName("app_name", ResType::STRING);
    start = CurrentTimeUsec();
    auto idValues = pResource->GetIdValues(id);
    cost = CurrentTimeUsec() - start;
    EXPECT_EQ(static_cast<size_t>(3), idValues->GetLimitPathsConst().size());
    RESMGR_HILOGD(RESMGR_TAG, "GetIdValues by id cost: %ld us.", cost);
    PrintIdValues(idValues);

    {
        auto limitPath = idValues->GetLimitPathsConst()[0];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "base");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "About");
    }
    {
        auto limitPath = idValues->GetLimitPathsConst()[1];
        EXPECT_TRUE(limitPath->GetResConfig()->ToString() == "en_Latn_US");
        EXPECT_TRUE(limitPath->GetIdItem()->name_ == "app_name");
        EXPECT_TRUE(limitPath->GetIdItem()->value_ == "App Name");
    }

    GetIdValuesByNameFuncTest002(pResource, id);
}

/*
 * load a hap, get a value which is ref
 * @tc.name: HapResourceFuncTest003
 * @tc.desc: Test GetIdValuesByName function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(HapResourceTest, HapResourceFuncTest003, TestSize.Level1)
{
    auto start = CurrentTimeUsec();
    std::shared_ptr<ResConfigImpl> rc = nullptr;
    auto pResource = HapResource::LoadFromIndex(FormatFullPath(g_resFilePath).c_str(), rc);
    auto cost = CurrentTimeUsec() - start;
    RESMGR_HILOGD(RESMGR_TAG, "load cost: %ld us.", cost);

    if (pResource == nullptr) {
        EXPECT_TRUE(false);
        return;
    }

    auto idv = pResource->GetIdValuesByName(std::string("integer_ref"), ResType::INTEGER);
    PrintIdValues(idv);

    idv = pResource->GetIdValuesByName(std::string("string_ref"), ResType::STRING);
    PrintIdValues(idv);

    // ref propagation
    idv = pResource->GetIdValuesByName(std::string("string_ref2"), ResType::STRING);
    PrintIdValues(idv);

    idv = pResource->GetIdValuesByName(std::string("boolean_ref"), ResType::BOOLEAN);
    PrintIdValues(idv);

    idv = pResource->GetIdValuesByName(std::string("color_ref"), ResType::COLOR);
    PrintIdValues(idv);

    idv = pResource->GetIdValuesByName(std::string("float_ref"), ResType::FLOAT);
    PrintIdValues(idv);

    // ref in array ,
    idv = pResource->GetIdValuesByName(std::string("intarray_1"), ResType::INTARRAY);
    PrintIdValues(idv);

    // "parent":   was ref too
    idv = pResource->GetIdValuesByName(std::string("child"), ResType::PATTERN);
    PrintIdValues(idv);
}

ResDesc *LoadFromHap(const char *hapPath, const std::shared_ptr<ResConfigImpl> defaultConfig)
{
    std::unique_ptr<uint8_t[]> buf;
    size_t bufLen;
    int32_t out = HapParser::ReadIndexFromFile(hapPath, buf, bufLen);
    if (out != OK) {
        RESMGR_HILOGE(RESMGR_TAG, "ReadIndexFromFile failed! retcode:%d", out);
        return nullptr;
    }
    RESMGR_HILOGD(RESMGR_TAG, "extract success, bufLen:%zu", bufLen);

    ResDesc *resDesc = new ResDesc();
    out = HapParser::ParseResHex(reinterpret_cast<char *>(buf.get()), bufLen, *resDesc, defaultConfig);
    if (out != OK) {
        delete (resDesc);
        RESMGR_HILOGE(RESMGR_TAG, "ParseResHex failed! retcode:%d", out);
        return nullptr;
    } else {
        RESMGR_HILOGD(RESMGR_TAG, "ParseResHex success:\n%s", resDesc->ToString().c_str());
    }
    // construct hapresource
    return resDesc;
}

/*
 * @tc.name: HapResourceFuncTest004
 * @tc.desc: Test HapParser::ReadIndexFromFile function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(HapResourceTest, HapResourceFuncTest004, TestSize.Level1)
{
    // 1. normal case
    ResDesc *resDesc = LoadFromHap(FormatFullPath("all.hap").c_str(), nullptr);
    ASSERT_TRUE(resDesc != nullptr);

    // 2. hap file exists, config.json does not exist
    resDesc = LoadFromHap(FormatFullPath("err-config.json-1.hap").c_str(), nullptr);
    ASSERT_TRUE(resDesc == nullptr);

    // 3. hap file exists, config.json error: missing "moduleName"
    resDesc = LoadFromHap(FormatFullPath("err-config.json-2.hap").c_str(), nullptr);
    ASSERT_TRUE(resDesc == nullptr);
}
}
