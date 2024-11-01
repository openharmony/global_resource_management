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
#ifndef OHOS_RESOURCE_MANAGER_RSTATE_H
#define OHOS_RESOURCE_MANAGER_RSTATE_H

namespace OHOS {
namespace Global {
namespace Resource {
enum RState {
    SUCCESS = 0,
    NOT_SUPPORT_SEP = 1,
    INVALID_BCP47_STR_LEN_TOO_SHORT = 2,
    INVALID_BCP47_LANGUAGE_SUBTAG = 3,
    INVALID_BCP47_SCRIPT_SUBTAG = 4,
    INVALID_BCP47_REGION_SUBTAG = 5,
    HAP_INIT_FAILED = 6,
    NOT_FOUND = 7,
    INVALID_FORMAT = 8,
    LOCALEINFO_IS_NULL = 9,
    NOT_ENOUGH_MEM = 10,
    ERROR = 10000,
    ERROR_CODE_INVALID_INPUT_PARAMETER = 401,
    ERROR_CODE_RES_ID_NOT_FOUND        = 9001001,
    ERROR_CODE_RES_NOT_FOUND_BY_ID     = 9001002,
    ERROR_CODE_RES_NAME_NOT_FOUND      = 9001003,
    ERROR_CODE_RES_NOT_FOUND_BY_NAME   = 9001004,
    ERROR_CODE_RES_PATH_INVALID        = 9001005,
    ERROR_CODE_RES_REF_TOO_MUCH        = 9001006,
    ERROR_CODE_RES_ID_FORMAT_ERROR     = 9001007,
    ERROR_CODE_RES_NAME_FORMAT_ERROR   = 9001008,
    ERROR_CODE_SYSTEM_RES_MANAGER_GET_FAILED = 9001009,
    ERROR_CODE_OVERLAY_RES_PATH_INVALID = 9001010,
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif