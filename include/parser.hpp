//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include "lexer.hpp"

namespace Coral
{
    enum class IntentType : u8
    {
        I_INVALID = 0u,

        I_INITIALIZER = 1u,
        I_ASSIGNER = 2u,
        I_RETURNER = 3u,

        I_FUNCTION = 4u
    };

    //An intent is like a question - what is the user trying to do?
    //Do they have a function body, a block body,
    //an initializer line, an assigner line or a returner line? 
    struct Intent
    {
        //what does this intent do
        vector<IntentType> types{};
        //what are the contents of this intent
        vector<Token> tokens{};

        //child intents of this intent
        vector<Intent> intent{};
    };

    class Parser
    {
        static vector<Intent> Parse(const vector<TokenData>& tokenData);
    };
}