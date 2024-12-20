/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "hisysevent_adapter.h"
#include "hisysevent.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace Global {
namespace Resource {
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;

void ReportInitResourceManagerFail(const std::string& bundleName, const std::string& errMsg)
{
    int ret = HiSysEventWrite(HiSysEventNameSpace::Domain::GLOBAL_RESMGR, "INIT_RESMGR_FAILED",
        HiSysEventNameSpace::EventType::FAULT,
        "BUNDLENAME", bundleName,
        "ERROR_MSG", errMsg);
    if (ret != 0) {
        RESMGR_HILOGE(RESMGR_TAG, "HiSysEventWrite failed! ret %{public}d, bundleName %{public}s, errMsg %{public}s",
            ret, bundleName.c_str(), errMsg.c_str());
    }
}

void ReportGetResourceByIdFail(uint32_t resId, const std::string& result, const std::string& errMsg)
{
    int ret = HiSysEventWrite(HiSysEventNameSpace::Domain::GLOBAL_RESMGR, "GET_RES_BY_ID_FAILED",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "ID", resId,
        "RESULT", result,
        "ERROR_MSG", errMsg);
    if (ret != 0) {
        RESMGR_HILOGE(RESMGR_TAG,
            "HiSysEventWrite failed! ret %{public}d, resId %{public}u, result %{public}s, errMsg %{public}s.",
            ret, resId, result.c_str(), errMsg.c_str());
    }
}

void ReportGetResourceByNameFail(const std::string& resName, const std::string& result, const std::string& errMsg)
{
    int ret = HiSysEventWrite(HiSysEventNameSpace::Domain::GLOBAL_RESMGR, "GET_RES_BY_NAME_FAILED",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "NAME", resName,
        "RESULT", result,
        "ERROR_MSG", errMsg);
    if (ret != 0) {
        RESMGR_HILOGE(RESMGR_TAG,
            "HiSysEventWrite failed! ret %{public}d, resName %{public}s, result %{public}s, errMsg %{public}s",
            ret, resName.c_str(), result.c_str(), errMsg.c_str());
    }
}

void ReportAddResourcePathFail(const char* resourcePath, const std::string& errMsg)
{
    int ret = HiSysEventWrite(HiSysEventNameSpace::Domain::GLOBAL_RESMGR, "ADD_RES_PATH_FAILED",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "PATH", resourcePath,
        "ERROR_MSG", errMsg);
    if (ret != 0) {
        RESMGR_HILOGE(RESMGR_TAG,
            "HiSysEventWrite failed! ret %{public}d, resourcePath %{public}s, errMsg %{public}s.",
            ret, resourcePath, errMsg.c_str());
    }
}
} // Resource
} // Global
} // OHOS