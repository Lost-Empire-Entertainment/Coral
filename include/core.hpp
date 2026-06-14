//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace Coral
{
    using std::vector;
    using std::string;
    using std::string_view;
    using std::filesystem::path;

    using u8 = uint8_t;
    using u16 = uint16_t;

    enum class Standard : u8
    {
        //unassigned or missing standard version
        S_INVALID = 0u,

        //Coral 2026
        S_CR26 = 1u,

        //always the highest possible value
        S_CR_LATEST = 255u
    };

    enum class Target : u8
    {
        T_INVALID = 0u,

        T_RELEASE = 1u,
        T_DEBUG = 2u,
        T_RELDEBUG = 3u,
        T_MINSIZEREL = 4u
    };

    enum class Platform : u8
    {
        P_INVALID = 0u,

        P_CURRENT = 1u,

        P_WINDOWS = 2u,
        P_LINUX = 3u,
        
        //currently unsupported due to limited testing capabilities
        //P_MAC = 4u
    };

    enum class Architecture : u8
    {
        A_INVALID = 0u,

        A_CURRENT = 1u,

        A_X64 = 2u,
        A_X86 = 3u,

        //currently unsupported due to limited testing capabilities
        //A_ARM = 4u
    };

    struct PreParseData
    {
        path outputPath{};
        vector<path> coralScripts{};
    };

    class CoralCore
    {
    public:
        static bool IsVerbose();
        static u16 GetJobsCount();
        
        static Standard GetStandard();
        static Standard GetLatestStandard();

        static Target GetTarget();
        static Platform GetPlatform();
        static Architecture GetArchitecture();

        static void Compile(const vector<string>& params);
    };
}