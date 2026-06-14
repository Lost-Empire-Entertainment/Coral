//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <cstdint>
#include <string_view>

namespace Coral
{
    using std::string_view;

    using u16 = uint16_t;
    using u32 = uint32_t;

    enum class ErrorType : u16
    {
        E_INVALID = 0u,

        //
        // 0*** - errors in core.cpp
        //

        //Cannot add ... more than once
        E_0001 = 1u,
        //No value was passed after ...
        E_0002 = 2u,

        //Jobs parameter expects atleast one job
        E_0003 = 3u,
        //Jobs parameter value ... exceeds u16 bounds
        E_0004 = 4u,
        //Failed to cast jobs value to integer
        E_0005 = 5u,

        //Output path already exists
        E_0006 = 6u,
        //... was not found
        E_0007 = 7u,
        //Failed to access path ...
        E_0008 = 8u,
        //No valid Coral scripts were found
        E_0009 = 9u,

        //
        // 1*** - errors in lexer.cpp
        //

        //Failed to read lines from Coral script
        E_1001 = 1001u,

        //Found duplicate multiline comment start
        E_1002 = 1002u,
        //Found multiline comment end without existing multiline comment start
        E_1003 = 1003u,
        //Script ended without multiline command end
        E_1004 = 1004u,

        //Found unsupported exclamation structure in token ...
        E_1005 = 1005u,
        //Found invalid operator ...
        E_1006 = 1006u,
        //Found ... symbol in an unsupported position in token ...
        E_1007 = 1007u,
        //Value ... cannot contain more than one dot
        E_1008 = 1008u,

        //
        // 2*** - errors in parser.cpp
        //

        //
        // 3*** - errors in transpiler.cpp
        //

        //
        // 4*** - 9*** errors in clang
        //
    };

    class Error
    {
    public:
        static void CloseOnError(
            string_view message,
            string_view target,
            ErrorType errorType,
            u32 linePos = UINT32_MAX,
            u32 charPos = UINT32_MAX);
    };
}