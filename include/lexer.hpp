//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.


#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

namespace Coral
{
    using std::string;
    using std::vector;
    using std::filesystem::path;

    using u8 = uint8_t;
    using u32 = uint32_t;

    enum class TokenType : u8
    {
        //default unassigned value
        T_INVALID = 0u,

        //value to any variable
        T_VALUE = 1u,

        //the operator used for the action, like u8 myvar = 1
        T_OPERATOR = 2u,

        //any known variable type like u8
        T_VARIABLE = 3u,

        //variable type of what the function returns
        T_RETURN = 4u,

        //start of the codeblock, like func
        T_START = 5u,

        //end of the code block, like endfunc
        T_END = 6u,

        //name of the function or variable
        T_IDENTIFIER = 7u,

        //visiblity specifier of the function or variable
        T_VISIBILITY = 8u,

        //mutability specifier of the variable, not applicable to functions
        T_MUTABILITY = 9u,

        //jump statement meant to break out of the current scope, like return, break or continue
        T_JUMP = 10u,
    };

    struct Token
    {
        u32 line{};
        u32 pos{};
        string token{};
        TokenType tokenType{};
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
    };
}