//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <cstdint>
#include <unordered_map>

#include "log_utils.hpp"
#include "core_utils.hpp"

#include "error.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaHeaders::KalaCore::EnumHash;
using KalaHeaders::KalaCore::EnumToString;

using Coral::ErrorType;

using std::string;
using std::string_view;
using std::to_string;
using std::unordered_map;

constexpr string_view e_0001 = "E0001";
constexpr string_view e_0002 = "E0002";
constexpr string_view e_0003 = "E0003";
constexpr string_view e_0004 = "E0004";
constexpr string_view e_0005 = "E0005";
constexpr string_view e_0006 = "E0006";
constexpr string_view e_0007 = "E0007";
constexpr string_view e_0008 = "E0008";
constexpr string_view e_0009 = "E0009";

constexpr string_view e_1001 = "E1001";
constexpr string_view e_1002 = "E1002";
constexpr string_view e_1003 = "E1003";
constexpr string_view e_1004 = "E1004";
constexpr string_view e_1005 = "E1005";
constexpr string_view e_1006 = "E1006";

static unordered_map<ErrorType, string_view, EnumHash<ErrorType>> errorTypes
{
    { ErrorType::E_0001, e_0001 },
    { ErrorType::E_0002, e_0002 },
    { ErrorType::E_0003, e_0003 },
    { ErrorType::E_0004, e_0004 },
    { ErrorType::E_0005, e_0005 },
    { ErrorType::E_0006, e_0006 },
    { ErrorType::E_0007, e_0007 },
    { ErrorType::E_0008, e_0008 },
    { ErrorType::E_0009, e_0009 },

    { ErrorType::E_1001, e_1001 },
    { ErrorType::E_1002, e_1002 },
    { ErrorType::E_1003, e_1003 },
    { ErrorType::E_1004, e_1004 },
    { ErrorType::E_1005, e_1005 },
    { ErrorType::E_1006, e_1006 }
};

namespace Coral
{
    void Error::CloseOnError(
        string_view message,
        string_view target,
        ErrorType errorType,
        u32 linePos,
        u32 charPos)
    {
        string_view out{};
        EnumToString(errorType, errorTypes, out);

        string msg = "\n        [ " + string(out) + " ] " + string(message);

        if (linePos != UINT32_MAX
            && charPos != UINT32_MAX)
        {
            msg += "\n        --- line: " + to_string(linePos) + ", char: " + to_string(charPos);
        }

        Log::Print(
            msg,
            target,
            LogType::LOG_ERROR,
            2);

        exit(1);
    }
}