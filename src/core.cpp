//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>

#include "log_utils.hpp"
#include "core_utils.hpp"
#include "file_utils.hpp"

#include "core.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaHeaders::KalaCore::ContainsValue;
using KalaHeaders::KalaCore::RemoveDuplicates;

using KalaHeaders::KalaFile::ResolveAnyPath;

using std::string_view;
using std::filesystem::path;
using std::filesystem::current_path;
using std::filesystem::exists;
using std::filesystem::is_regular_file;
using std::filesystem::is_directory;
using std::filesystem::is_empty;
using std::filesystem::recursive_directory_iterator;

constexpr string_view param_verbose = "--verbose";
constexpr string_view param_release = "--release";
constexpr string_view param_target = "--target";

static void ExitOnError(string_view message)
{
    Log::Print(
        message,
        "CORE",
        LogType::LOG_ERROR,
        2);

    exit(1);
}

namespace Coral
{
    static bool isVerbose{};

    bool CoralCore::IsVerbose() { return isVerbose; }

    void CoralCore::Compile(const vector<string> &params)
    {
        string release{};
        path targetPath{};
        vector<path> coralScripts{};

        if (!ContainsValue(params, param_release))
        {
            ExitOnError("Did not find coral language release parameter! You must pass --release with a valid value after it.");
        }

        if (!ContainsValue(params, param_target))
        {
            ExitOnError("Did not find coral executable target path parameter! You must pass --target with a valid path after it.");
        }

        string relativeDir = current_path().string();

        //starts at 1 to skip --compile
        for (size_t i = 1; i < params.size(); i++)
        {
            string_view p = params[i];

            //Log::Print("@@@@@ " + string(p));

            if (p == param_verbose)
            {
                if (isVerbose)
                {
                    ExitOnError("Cannot add more than one verbose parameter!");
                }

                isVerbose = true;

                continue;
            }

            if (p == param_release)
            {
                if (!release.empty())
                {
                    ExitOnError("Cannot add more than one release parameter!");
                }

                if (i + 1 == params.size())
                {
                    ExitOnError("No value was passed after release parameter! You must pass a value after it.");
                }
                
                i++;
                release = params[i];

                continue;
            }

            if (p == param_target)
            {
                if (!targetPath.empty())
                {
                    ExitOnError("Cannot add more than one target parameter!");
                }

                if (i + 1 == params.size())
                {
                    ExitOnError("No value was passed after target path parameter! You must pass a path after it.");
                }

                i++;
                
                if (exists(params[i]))
                {
                    ExitOnError("Target path '" + params[i] + "' already exists!");
                }

                targetPath = params[i];

                continue;
            }

            vector<path> outPath{};
            string errorMessage = ResolveAnyPath(
                p,
                relativeDir,
                outPath);

            if (!errorMessage.empty())
            {
                ExitOnError("Failed to access path '" + string(p) + "'! Reason: " + errorMessage);
            }

            for (const auto& pp : outPath)
            {
                if (is_regular_file(pp))
                {
                    if (!path(pp).string().ends_with(".coral"))
                    {
                        if (isVerbose)
                        {
                            Log::Print(
                                "Skipped invalid file '" + path(pp).string() + "'.",
                                "CORE",
                                LogType::LOG_VERBOSE);
                        }

                        continue;
                    }

                    if (is_empty(pp))
                    {
                        Log::Print(
                            "Skipped empty coral script '" + path(pp).string() + "'.",
                            "CORE",
                            LogType::LOG_WARNING);

                        continue;
                    }
                    
                    coralScripts.push_back(pp);

                    if (isVerbose)
                    {
                        Log::Print(
                            "Found valid coral script '" + path(pp).string() + "'.",
                            "CORE",
                            LogType::LOG_VERBOSE);
                    }
                }
                else if (is_directory(pp))
                {
                    for (const auto& dir : recursive_directory_iterator(pp))
                    {
                        if (is_regular_file(dir))
                        {
                            if (!path(dir).string().ends_with(".coral"))
                            {
                                if (isVerbose)
                                {
                                    Log::Print(
                                        "Skipped invalid file '" + path(dir).string() + "'.",
                                        "CORE",
                                        LogType::LOG_VERBOSE);
                                }

                                continue;
                            }

                            if (is_empty(dir))
                            {
                                Log::Print(
                                    "Skipped empty coral script '" + path(dir).string() + "'.",
                                    "CORE",
                                    LogType::LOG_WARNING);

                                continue;
                            }

                            coralScripts.push_back(dir);

                            if (isVerbose)
                            {
                                Log::Print(
                                    "Found valid coral script '" + path(dir).string() + "'.",
                                    "CORE",
                                    LogType::LOG_VERBOSE);
                            }
                        }
                    }
                }
            }
        }

        RemoveDuplicates(coralScripts);
        if (coralScripts.empty())
        {
            ExitOnError("No valid coral scripts were found! There is nothing to compile.");
        }
    }
}