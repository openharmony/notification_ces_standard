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

#include "common_event_command.h"

#include <getopt.h>
#include <charconv>
#include <string_view>

#include "common_event.h"
#include "common_event_constant.h"
#include "common_event_manager.h"
#include "event_log_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
namespace {
const std::string SHORT_OPTIONS = "he:asoc:d:u:";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"all", no_argument, nullptr, 'a'},
    {"event", required_argument, nullptr, 'e'},
    {"sticky", no_argument, nullptr, 's'},
    {"ordered", no_argument, nullptr, 'o'},
    {"code", required_argument, nullptr, 'c'},
    {"data", required_argument, nullptr, 'd'},
    {"user-id", required_argument, nullptr, 'u'},
};
const std::string INVALID_INT_MSG = "error: option requires a valid integer.\n";

bool ParseCemInt32(std::string_view text, int32_t &out)
{
    if (text.empty()) {
        return false;
    }
    int32_t value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc() || ptr != text.data() + text.size()) {
        return false;
    }
    out = value;
    return true;
}
}  // namespace

CommonEventCommand::CommonEventCommand(int argc, char *argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{
    EVENT_LOGI("enter");

    for (int i = 0; i < argc_; i++) {
        EVENT_LOGI("argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
}

ErrCode CommonEventCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&CommonEventCommand::RunAsHelpCommand, this)},
        {"publish", std::bind(&CommonEventCommand::RunAsPublishCommand, this)},
        {"dump", std::bind(&CommonEventCommand::RunAsDumpCommand, this)},
    };

    return OHOS::ERR_OK;
}

ErrCode CommonEventCommand::CreateMessageMap()
{
    messageMap_ = {};

    return OHOS::ERR_OK;
}

ErrCode CommonEventCommand::init()
{
    EVENT_LOGI("enter");

    ErrCode result = OHOS::ERR_OK;

    if (!commonEventPtr_) {
        commonEventPtr_ = DelayedSingleton<CommonEvent>::GetInstance();
    }

    if (!commonEventPtr_) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode CommonEventCommand::RunAsHelpCommand()
{
    EVENT_LOGI("enter");

    resultReceiver_.append(HELP_MSG);

    return OHOS::ERR_OK;
}

ErrCode CommonEventCommand::RunAsPublishCommand()
{
    EVENT_LOGI("enter");

    ErrCode result = OHOS::ERR_OK;

    int option = -1;
    std::int32_t userId = ALL_USER;
    int counter = 0;

    bool isSticky = false;
    bool isOrdered = false;

    std::string action = "";
    int code = 0;
    std::string data = "";

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        EVENT_LOGI("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if ((optind < 0) || (optind > argc_)) {
            return OHOS::ERR_INVALID_VALUE;
        }

        for (int i = 0; i < argc_; i++) {
            EVENT_LOGI("argv_[%{public}d]: %{public}s", i, argv_[i]);
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'cem publish' with no option: cem publish
                    // 'cem publish' with a wrong argument: cem publish xxx
                    EVENT_LOGI("'cem publish' with no option.");

                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");

                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'e': {
                    // 'cem publish -e' with no argument: cem publish -e
                    // 'cem publish --event' with no argument: cem publish --event
                    EVENT_LOGI("'cem publish -e' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'c': {
                    // 'cem publish -e <name> -c' with no argument: cem publish -e <name> -c
                    // 'cem publish --event <name> -c' with no argument: cem publish --event -c
                    EVENT_LOGI("'cem publish -e <name> -c' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'cem publish -e <name> -d' with no argument: cem publish -e <name> -d
                    // 'cem publish --event <name> -d' with no argument: cem publish --event -d
                    EVENT_LOGI("'cem publish -e <name> -d' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u':
                    // 'cem publish -e <name> -u' with no argument: cem publish -e <name> -d
                    // 'cem publish --event <name> -d' with no argument: cem publish --event -d
                    EVENT_LOGI("'cem publish -e <name> -u' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                case 0: {
                    // 'cem publish' with an unknown option: cem publish --x
                    // 'cem publish' with an unknown option: cem publish --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem publish' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'cem publish' with an unknown option: cem publish -x
                    // 'cem publish' with an unknown option: cem publish -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem publish' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'cem publish -h'
                // 'cem publish --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'e': {
                // 'cem publish -e <name>'
                // 'cem publish --event <name>'
                action = optarg;
                break;
            }
            case 's': {
                // 'cem publish -e <name> -s '
                // 'cem publish -e <name> --sticky'
                isSticky = true;
                break;
            }
            case 'o': {
                // 'cem publish -e <name> -o '
                // 'cem publish -e <name> --ordered'
                isOrdered = true;
                break;
            }
            case 'c': {
                // 'cem publish -e <name> -c 1024 '
                // 'cem publish -e <name> --code 1024'
                int32_t parsedCode = 0;
                if (!ParseCemInt32(optarg != nullptr ? optarg : "", parsedCode)) {
                    resultReceiver_.append(INVALID_INT_MSG);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                code = parsedCode;
                break;
            }
            case 'd': {
                // 'cem publish -e <name> -d 1024 '
                // 'cem publish -e <name> --data 1024'
                data = optarg;
                break;
            }
            case 'u': {
                // 'cem publish -e <name> -u 100'
                // 'cem publish --event <name> -u 100'
                if (!ParseCemInt32(optarg != nullptr ? optarg : "", userId)) {
                    resultReceiver_.append(INVALID_INT_MSG);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                break;
            }
            case 0: {
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if ((resultReceiver_ == "") && (action.size() == 0)) {
            // 'cem publish ...' with no event option
            EVENT_LOGI("'cem publish' with no event option.");

            resultReceiver_.append(HELP_MSG_NO_EVENT_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_PUBLISH);
    } else {
        /* Publish */

        // make a want
        Want want;
        want.SetAction(action);

        // make common event data
        CommonEventData commonEventData;
        commonEventData.SetWant(want);

        // set code
        commonEventData.SetCode(code);

        // set data
        commonEventData.SetData(data);

        // make publish info
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(isSticky);
        publishInfo.SetOrdered(isOrdered);

        // publish the common event
        std::shared_ptr<CommonEventSubscriber> subscriber = nullptr;
        bool publishResult = commonEventPtr_->PublishCommonEventAsUser(
            commonEventData, publishInfo, subscriber, userId);
        if (publishResult) {
            resultReceiver_ = STRING_PUBLISH_COMMON_EVENT_OK + "\n";
        } else {
            resultReceiver_ = STRING_PUBLISH_COMMON_EVENT_NG + "\n";
        }
    }

    return result;
}

ErrCode CommonEventCommand::RunAsDumpCommand()
{
    EVENT_LOGI("enter");

    ErrCode result = OHOS::ERR_OK;
    std::vector<std::string> dumpResults;

    std::string action = "";
    std::int32_t userId = ALL_USER;

    int option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

    EVENT_LOGI("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

    if ((optind < 0) || (optind > argc_)) {
        return OHOS::ERR_INVALID_VALUE;
    }

    for (int i = 0; i < argc_; i++) {
        EVENT_LOGI("argv_[%{public}d]: %{public}s", i, argv_[i]);
    }

    switch (option) {
        case 'h': {
            // 'cem dump -h'
            // 'cem dump --help'
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 'a': {
            // 'cem dump -a'
            // 'cem dump --all'
            break;
        }
        case 'e': {
            // 'cem dump -e <name>'
            // 'cem dump --event <name>'
            action = optarg;
            break;
        }
        case 'u': {
            // 'cem dump -e <name> -u 100'
            // 'cem dump --event <name> -u 100'
            if (!ParseCemInt32(optarg != nullptr ? optarg : "", userId)) {
                resultReceiver_.append(INVALID_INT_MSG);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            break;
        }
        case '?': {
            switch (optopt) {
                case 'e': {
                    // 'cem dump -e' with no argument: cem dump -e
                    // 'cem dump --event' with no argument: cem dump --event
                    EVENT_LOGI("'cem dump -s' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u':
                    // 'cem dump -e' with no argument: cem dump -e
                    // 'cem dump --event' with no argument: cem dump --user-id
                    EVENT_LOGI("'cem dump -u' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                case 0: {
                    // 'cem dump' with an unknown option: cem dump --x
                    // 'cem dump' with an unknown option: cem dump --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem dump' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'cem dump' with an unknown option: cem dump -x
                    // 'cem dump' with an unknown option: cem dump -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem dump' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        default: {
            if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                // 'cem dump' with no option: cem dump
                // 'cem dump' with a wrong argument: cem dump xxx
                EVENT_LOGI("'cem dump' with no option.");

                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP);
    } else {
        // dump state
        bool dumpResult = commonEventPtr_->DumpState(action, userId, dumpResults);
        if (dumpResult) {
            for (auto it : dumpResults) {
                resultReceiver_ += it + "\n";
            }
        } else {
            resultReceiver_ = STRING_DUMP_COMMON_EVENT_NG + "\n";
        }
    }

    return result;
}
}  // namespace EventFwk
}  // namespace OHOS
