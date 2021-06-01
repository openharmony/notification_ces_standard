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
#ifndef TEST_CONFIG_PARSER_H
#define TEST_CONFIG_PARSER_H

#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace OHOS {

const std::string STRESS_TEST_CONFIG_FILE_PATH{"/testconfig/stressconfig.json"};

const std::string STRESS_TEST_AMS_KEY{"AMS"};
const std::string STRESS_TEST_BMS_KEY{"BMS"};
const std::string STRESS_TEST_CES_KEY{"CES"};

struct StressTestLevel {
    int32_t AMSLevel = 0;
    int32_t BMSLevel = 0;
    int32_t CESLevel = 0;
};

class TestConfigParser {
public:
    void ParseFromFile4StressTest(const std::string &path, StressTestLevel &stlevel)
    {
        if (path.empty()) {
            std::cout << __FUNCTION__ << " invalid file path, check!" << std::endl;
            return;
        }

        nlohmann::json jsonObj;
        std::ifstream(path) >> jsonObj;

        const auto &jsonObjEnd = jsonObj.end();
        if (jsonObj.find(STRESS_TEST_AMS_KEY) != jsonObjEnd) {
            jsonObj.at(STRESS_TEST_AMS_KEY).get_to(stlevel.AMSLevel);
        }

        if (jsonObj.find(STRESS_TEST_BMS_KEY) != jsonObjEnd) {
            jsonObj.at(STRESS_TEST_BMS_KEY).get_to(stlevel.BMSLevel);
        }

        if (jsonObj.find(STRESS_TEST_CES_KEY) != jsonObjEnd) {
            jsonObj.at(STRESS_TEST_CES_KEY).get_to(stlevel.CESLevel);
        }
    }
};

}  // namespace OHOS

#endif  // TEST_CONFIG_PARSER_H