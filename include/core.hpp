//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <vector>
#include <string>

namespace Coral
{
    using std::vector;
    using std::string;

    class CoralCore
    {
    public:
        static bool IsVerbose();

        static void Compile(const vector<string>& params);
    };
}