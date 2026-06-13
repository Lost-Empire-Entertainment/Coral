//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>
#include <variant>

namespace Coral
{
    using std::string;
    using std::string_view;
    using std::vector;
    using std::filesystem::path;
    using std::variant;
    using std::monostate;

    using u8 = uint8_t;
    using u32 = uint32_t;

    enum class TokenType : u8
    {
        //default unassigned value
        T_INVALID = 0u,

        //name of the function or variable
        T_IDENTIFIER = 1u,

        //value to any variable
        T_VALUE = 2u,

        //the operator used for the action, like u8 myvar = 1
        T_OPERATOR = 3u,

        //any known variable type like u8
        T_VARIABLE = 4u,

        //variable type of what the function returns
        T_RETURN = 5u,

        //start of the codeblock, like func
        T_START = 6u,

        //end of the code block, like endfunc
        T_END = 7u,

        //visiblity specifier of the function or variable
        T_VISIBILITY = 8u,

        //mutability specifier of the variable, not applicable to functions
        T_MUTABILITY = 9u,

        //jump statement meant to break out of the current scope, like return, break or continue
        T_JUMP = 10u,
    };

    enum class Token_Operator : u8
    {
        O_INVALID = 0u,

        O_ASSIGN = 1u,
        O_RETURN = 2u,

        O_AND = 3u,
        O_EQUALS = 4u,
        O_NOT_EQUALS = 5u,
        O_PLUS_EQUALS = 6u,
        O_MINUS_EQUALS = 7u,
        O_MULTIPLY_EQUALS = 8u,
        O_DIVIDE_EQUALS = 9u,

        O_MORE_THAN = 10u,
        O_LESS_THAN = 11u,
        O_MORE_AND_EQUAL = 12u,
        O_LESS_AND_EQUAL = 13u,

        O_ADD = 14u,
        O_SUBTRACT = 15u,
        O_MULTIPLY = 16u,
        O_DIVIDE = 17u
    };

    enum class Token_Variable : u8
    {
        V_INVALID = 0u,

        V_INT = 1u,
        V_FLOAT = 2u,
        V_DOUBLE = 3u,
        V_BOOL = 4u
    };

    enum class Token_Return : u8
    {
        R_INVALID = 0u,

        R_INT = 1u,
        R_FLOAT = 2u,
        R_DOUBLE = 3u,
        R_BOOL = 4u,

        R_NULL = 5u
    };

    enum class Token_Start : u8
    {
        S_INVALID = 0u,

        S_MAIN = 1u,
        S_FUNC = 2u,
        S_BLOCK = 3u,

        S_WHILE = 4u,
        S_FOR = 5u
    };

    enum class Token_End : u8
    {
        E_INVALID = 0u,

        E_ENDMAIN = 1u,
        E_ENDFUNC = 2u,
        E_ENDBLOCK = 3u,

        E_ENDWHILE = 4u,
        E_ENDFOR = 5u
    };

    enum class Token_Visibility : u8
    {
        V_INVALID = 0u,

        V_PUBLIC = 1u,
        V_PRIVATE = 2u
    };

    enum class Token_Mutability : u8
    {
        M_INVALID = 0u,

        M_READONLY = 1u,
        M_LOCKED = 2u
    };

    enum class Token_Jump : u8
    {
        J_INVALID = 0u,

        J_RETURN = 1u,
        J_BREAK = 2u,
        J_CONTINUE = 3u
    };

    using TokenValue = variant<
        monostate,
        Token_Operator,
        Token_Variable,
        Token_Return,
        Token_Start,
        Token_End,
        Token_Visibility,
        Token_Mutability,
        Token_Jump>;

    struct Token
    {
        u32 line{};
        u32 pos{};
        string token{};
        TokenType tokenType{};
        TokenValue tokenValue{};
    };

    struct TokenData
    {
        vector<Token> tokens{};
        path scriptPath{};
    };

    class Lexer
    {
    public:
        static TokenData Tokenize(const path& script);

        static string_view TokenValueToString(const TokenValue& value);
    };
}