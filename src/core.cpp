//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <unordered_map>
#include <atomic>
#include <thread>

#include "core_utils.hpp"
#include "file_utils.hpp"
#include "log_utils.hpp"

#include "core.hpp"
#include "lexer.hpp"
#include "error.hpp"

using KalaHeaders::KalaCore::StringToEnum;
using KalaHeaders::KalaCore::EnumToString;
using KalaHeaders::KalaCore::ContainsValue;
using KalaHeaders::KalaCore::EnumHash;

using KalaHeaders::KalaFile::ResolveAnyPath;

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using Coral::CoralCore;
using Coral::PreParseData;
using Coral::Standard;
using Coral::Target;
using Coral::Platform;
using Coral::Architecture;
using Coral::TokenData;
using Coral::Lexer;
using Coral::Error;
using Coral::ErrorType;

using std::vector;
using std::string;
using std::string_view;
using std::to_string;
using std::filesystem::path;
using std::filesystem::current_path;
using std::filesystem::exists;
using std::filesystem::is_regular_file;
using std::filesystem::is_directory;
using std::filesystem::is_empty;
using std::filesystem::recursive_directory_iterator;
using std::exception;
using std::unordered_map;
using std::atomic;
using std::thread;
using std::memory_order_relaxed;

using u16 = uint16_t;

constexpr string_view param_verbose      = "--verbose";
constexpr string_view param_standard     = "--standard";
constexpr string_view param_target       = "--target";
constexpr string_view param_platform     = "--platform";
constexpr string_view param_architecture = "--architecture";
constexpr string_view param_output       = "--output";
constexpr string_view param_jobs         = "--jobs";

constexpr string_view standard_cr26     = "cr26";
constexpr string_view standard_crlatest = "crlatest";

constexpr string_view target_release    = "release";
constexpr string_view target_debug      = "debug";
constexpr string_view target_reledebug  = "reldebug";
constexpr string_view target_minsizerel = "minsizerel";

constexpr string_view platform_current = "current";
constexpr string_view platform_windows = "windows";
constexpr string_view platform_linux   = "linux";
//constexpr string_view platform_mac = "mac";

constexpr string_view architecture_current = "current";
constexpr string_view architecture_x64 = "x64";
constexpr string_view architecture_x86 = "x86";
//constexpr string_view architecture_arm = "arm";

static bool isVerbose{};
static u16 jobs{};
static Standard standard{};
constexpr Standard standard_latest = Standard::S_CR26;
static Target target{};
static Platform platform{};
static Architecture architecture{};

static const unordered_map<Standard, string_view, EnumHash<Standard>> standards =
{
    { Standard::S_CR26,      standard_cr26 },
    { Standard::S_CR_LATEST, standard_crlatest }
};

static const unordered_map<Target, string_view, EnumHash<Target>> targets =
{
    { Target::T_RELEASE,    target_release },
    { Target::T_DEBUG,      target_debug },
    { Target::T_RELDEBUG,   target_reledebug },
    { Target::T_MINSIZEREL, target_minsizerel }
};

static const unordered_map<Platform, string_view, EnumHash<Platform>> platforms =
{
    { Platform::P_CURRENT, platform_current },
    { Platform::P_WINDOWS, platform_windows },
    { Platform::P_LINUX,   platform_linux },
    //{ Platform::P_MAC,         platform_mac }
};

static const unordered_map<Architecture, string_view, EnumHash<Architecture>> architectures =
{
    { Architecture::A_CURRENT, architecture_current },
    { Architecture::A_X64, architecture_x64 },
    { Architecture::A_X86, architecture_x86 },
    //{ Architecture::A_ARM, architecture_arm }
};

static PreParseData PreParse(const vector<string>& params)
{
    bool hasVerbose{};
    bool hasJobs{};
    PreParseData pdata{};

    if (ContainsValue(params, param_verbose)) isVerbose = true;

    string relativeDir = current_path().string();

    //starts at 1 to skip --compile
    for (size_t i = 1; i < params.size(); i++)
    {
        string_view p = params[i];

        if (p == param_verbose)
        {
            if (hasVerbose)
            {
                Error::CloseOnError(
                    "Cannot add verbose more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }

            hasVerbose = true;
            isVerbose = true;

            continue;
        }

        if (p == param_jobs)
        {
            if (jobs > 0)
            {
                Error::CloseOnError(
                    "Cannot add jobs more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }
            if (i + 1 == params.size())
            {
                Error::CloseOnError(
                    "No value was passed after jobs!",
                    "CORE",
                    ErrorType::E_0002);
            }
            
            i++;
            p = params[i];

            auto cast_to_u16 = [&p]() -> u16
                {
                    try 
                    {
                        int value = stoi(string(p));

                        if (value > 0 && value <= UINT16_MAX) return value;
                        else if (value == 0)
                        {
                            Error::CloseOnError(
                                "Jobs parameter expects atleast one job!",
                                "CORE",
                                ErrorType::E_0003);
                        }
                        else
                        {
                            Error::CloseOnError(
                                "Jobs parameter value '" + string(p) + "' exceeds uint16 bounds!", 
                                "CORE",
                                ErrorType::E_0004);
                        }
                    }
                    catch (const exception& e)
                    {
                        Error::CloseOnError(
                            "Failed to cast jobs value to integer! Reason: " + string(e.what()), 
                            "CORE",
                            ErrorType::E_0005);
                    }

                    return 1;
                };

            jobs = cast_to_u16();

            if (isVerbose)
            {
                Log::Print(
                    "Set jobs count to '" + string(p) + "'.",
                    "CORE",
                    LogType::LOG_VERBOSE);
            }

            hasJobs = true;

            continue;
        }

        if (p == param_output)
        {
            if (!pdata.outputPath.empty())
            {
                Error::CloseOnError(
                    "Cannot add output path more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }

            if (i + 1 == params.size())
            {
                Error::CloseOnError(
                    "No value was passed after output path parameter!",
                    "CORE",
                    ErrorType::E_0002);
            }

            i++;
            p = params[i];
            
            if (exists(p))
            {
                Error::CloseOnError(
                    "Output path '" + string(p) + "' already exists!",
                    "CORE",
                    ErrorType::E_0006);
            }

            pdata.outputPath = p;

            continue;
        }

        if (p == param_standard)
        {
            if (standard != Standard::S_INVALID)
            {
                Error::CloseOnError(
                    "Cannot add standard more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }
            if (i + 1 == params.size())
            {
                Error::CloseOnError(
                    "No value was passed after standard!",
                    "CORE",
                    ErrorType::E_0002);
            }
            
            i++;
            p = params[i];

            if (!ContainsValue(standards, p))
            {
                Error::CloseOnError(
                    "Standard '" + string(p) + "' was not found!",
                    "CORE",
                    ErrorType::E_0007);
            }

            Standard out{};
            StringToEnum(p, standards, out);
            standard = out;

            if (isVerbose)
            {
                Log::Print(
                    "Set standard to '" + string(p) + "'.",
                    "CORE",
                    LogType::LOG_VERBOSE);
            }

            continue;
        }

        if (p == param_target)
        {
            if (target != Target::T_INVALID)
            {
                Error::CloseOnError(
                    "Cannot add target more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }
            if (i + 1 == params.size())
            {
                Error::CloseOnError(
                    "No value was passed after target!",
                    "CORE",
                    ErrorType::E_0002);
            }
            
            i++;
            p = params[i];

            if (!ContainsValue(targets, p))
            {
                Error::CloseOnError(
                    "Target '" + string(p) + "' was not found!",
                    "CORE",
                    ErrorType::E_0007);
            }

            Target out{};
            StringToEnum(p, targets, out);
            target = out;

            if (isVerbose)
            {
                Log::Print(
                    "Set target to '" + string(p) + "'.",
                    "CORE",
                    LogType::LOG_VERBOSE);
            }

            continue;
        }

        if (p == param_platform)
        {
            if (platform != Platform::P_INVALID)
            {
                Error::CloseOnError(
                    "Cannot add platform more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }
            if (i + 1 == params.size())
            {
                Error::CloseOnError(
                    "No value was passed after platform!",
                    "CORE",
                    ErrorType::E_0002);
            }
            
            i++;
            p = params[i];

            if (!ContainsValue(platforms, p))
            {
                Error::CloseOnError(
                    "Platform '" + string(p) + "' was not found!",
                    "CORE",
                    ErrorType::E_0007);
            }

            Platform out{};
            StringToEnum(p, platforms, out);
            platform = out;

            if (isVerbose)
            {
                Log::Print(
                    "Set platform to '" + string(p) + "'.",
                    "CORE",
                    LogType::LOG_VERBOSE);
            }

            continue;
        }

        if (p == param_architecture)
        {
            if (architecture != Architecture::A_INVALID)
            {
                Error::CloseOnError(
                    "Cannot add architecture more than once!",
                    "CORE",
                    ErrorType::E_0001);
            }
            if (i + 1 == params.size())
            {
                Error::CloseOnError(
                    "No value was passed after architecture!",
                    "CORE",
                    ErrorType::E_0002);
            }
            
            i++;
            p = params[i];

            if (!ContainsValue(architectures, p))
            {
                Error::CloseOnError(
                    "Architecture '" + string(p) + "' was not found!",
                    "CORE",
                    ErrorType::E_0007);
            }

            Architecture out{};
            StringToEnum(p, architectures, out);
            architecture = out;

            if (isVerbose)
            {
                Log::Print(
                    "Set architecture to '" + string(p) + "'.",
                    "CORE",
                    LogType::LOG_VERBOSE);
            }

            continue;
        }

        vector<path> resolvedPaths{};
        string errorMessage = ResolveAnyPath(
            p,
            relativeDir,
            resolvedPaths);

        if (!errorMessage.empty())
        {
            Error::CloseOnError(
                "Failed to access path '" + string(p) + "'! Reason: " + errorMessage,
                "CORE",
                ErrorType::E_0008);
        }

        for (const auto& rp : resolvedPaths)
        {
            if (is_regular_file(rp))
            {
                if (!path(rp).string().ends_with(".coral"))
                {
                    if (isVerbose)
                    {
                        Log::Print(
                            "Skipped unsupported file '" + path(rp).string() + "'.",
                            "CORE",
                            LogType::LOG_VERBOSE);
                    }

                    continue;
                }

                if (is_empty(rp))
                {
                    Log::Print(
                        "Skipped empty coral script '" + path(rp).string() + "'.",
                        "CORE",
                        LogType::LOG_WARNING);

                    continue;
                }

                if (ContainsValue(pdata.coralScripts, rp))
                {
                    Log::Print(
                        "Skipped duplicate coral script '" + path(rp).string() + "'.",
                        "CORE",
                        LogType::LOG_WARNING);

                    continue;
                }
                
                pdata.coralScripts.push_back(rp);

                if (isVerbose)
                {
                    Log::Print(
                        "Found valid coral script '" + path(rp).string() + "'.",
                        "CORE",
                        LogType::LOG_VERBOSE);
                }
            }
            else if (is_directory(rp))
            {
                for (const auto& dir : recursive_directory_iterator(rp))
                {
                    if (is_regular_file(dir))
                    {
                        if (!path(dir).string().ends_with(".coral"))
                        {
                            if (isVerbose)
                            {
                                Log::Print(
                                    "Skipped unsupported file '" + path(dir).string() + "'.",
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

                        if (ContainsValue(pdata.coralScripts, dir))
                        {
                            Log::Print(
                                "Skipped duplicate coral script '" + path(dir).string() + "'.",
                                "CORE",
                                LogType::LOG_WARNING);

                            continue;
                        }

                        pdata.coralScripts.push_back(dir);

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

    if (pdata.coralScripts.empty())
    {
        Error::CloseOnError(
            "No valid coral scripts were found! There is nothing to compile.",
            "CORE",
            ErrorType::E_0009);
    }

    if (standard == Standard::S_INVALID)
    {
        standard = Coral::CoralCore::GetLatestStandard();

        if (isVerbose)
        {
            string_view out{};
            EnumToString(standard, standards, out);

            Log::Print(
                "No standard was passed, defaulted to '" + string(out) + "'.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
    }

    if (pdata.outputPath.empty())
    {
        path output = "output";
#ifdef _WIN32
        output += ".exe";
#endif

        pdata.outputPath = current_path() / output;

        if (isVerbose)
        {
            Log::Print(
                "No output was passed, defaulted to '" + pdata.outputPath.string() + "'.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
    }

    if (target == Target::T_INVALID)
    {
        target = Target::T_RELEASE;

        if (isVerbose)
        {
            Log::Print(
                "No target was passed, defaulted to 'Release'.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
    }
    if (platform == Platform::P_INVALID)
    {
#ifdef _WIN32
        platform = Platform::P_WINDOWS;
        
        if (isVerbose)
        {
            Log::Print(
                "No platform was passed, defaulted to 'Windows' based off of current OS.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
#else
        platform = Platform::P_LINUX;

        if (isVerbose)
        {
            Log::Print(
                "No platform was passed, defaulted to 'Linux' based off of current OS.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
#endif
    }
    if (architecture == Architecture::A_INVALID)
    {
#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)
        architecture = Architecture::A_X64;

        if (isVerbose)
        {
            Log::Print(
                "No architecture was passed, defaulted to 'X64' based off of current CPU.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386)
        architecture = Architecture::A_X86;
        
        if (isVerbose)
        {
            Log::Print(
                "No architecture was passed, defaulted to 'X86' based off of current CPU.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
#endif
    }

    if (!hasJobs)
    {
#if _WIN32
        jobs = scast<u16>(GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
#else
        jobs = scast<u16>(sysconf(_SC_NPROCESSORS_ONLN));
#endif

        if (isVerbose)
        {
            Log::Print(
                "No jobs count was passed, defaulted to '" + to_string(jobs) + "' jobs.",
                "CORE",
                LogType::LOG_VERBOSE);
        }
    }

    return pdata;
}

namespace Coral
{
    bool CoralCore::IsVerbose() { return isVerbose; }
    u16 CoralCore::GetJobsCount() { return jobs; }

    Standard CoralCore::GetStandard() { return standard; }
    Standard CoralCore::GetLatestStandard() { return standard_latest; }

    Target CoralCore::GetTarget() { return target; }

    Platform CoralCore::GetPlatform() { return platform; }

    Architecture CoralCore::GetArchitecture() { return architecture; }

    void CoralCore::Compile(const vector<string> &params)
    {
        Log::Print(
            "Starting to parse parameters passed to Coral.",
            "CORE",
            LogType::LOG_INFO);

        PreParseData pdata = PreParse(params);

        auto tokenize = [&pdata]() -> vector<TokenData>
            {
                if (pdata.coralScripts.size() == 1) return { Lexer::Tokenize(pdata.coralScripts[0]) };

                u16 max_jobs = min(
                    scast<size_t>(jobs),
                    pdata.coralScripts.size());

                atomic<int> next{};
                vector<thread> workers{};
                workers.reserve(max_jobs);
                vector<TokenData> data(pdata.coralScripts.size());

                for (u16 i = 0; i < max_jobs; ++i)
                {
                    workers.emplace_back([
                        &next,
                        &pdata,
                        &data]
                        {
                            while (true)
                            {
                                size_t idx = next.fetch_add(1, memory_order_relaxed);
                                if (idx >= pdata.coralScripts.size()) break;

                                data[idx] = Lexer::Tokenize(pdata.coralScripts[idx]);
                            }
                        });
                }

                for (auto& w : workers) w.join();

                return data;
            };

        vector<TokenData> tdata = tokenize();
    }
}