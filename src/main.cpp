//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include "log_utils.hpp"

#include "kc_core.hpp"
#include "kc_command.hpp"

#include "core.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaCLI::Core;
using KalaCLI::CommandManager;

using Coral::CoralCore;

using std::vector;
using std::string;

static void AddExternalCommands()
{
    auto command_compile = [](const vector<string>& params) -> void
        {
            if (params.size() < 2)
            {
                Log::Print(
                    "Command 'compile' requires atleast one argument!",
                    "PARSE",
                    LogType::LOG_ERROR,
                2);

                return;
            }

            CoralCore::Compile(params);
        };
    auto command_version = [](const vector<string>& params) -> void
        {
            if (params.size() > 1)
            {
                Log::Print(
                    "Command 'version' does not allow any arguments!",
                    "PARSE",
                    LogType::LOG_ERROR,
                2);

                return;
            }

            Log::Print("Coral compiler pre-release 1");
        };

    CommandManager::AddCommand(
        {
            .primaryParam = "compile",
            .description =
                "Compile selected .coral files directly or from passed directories into an executable at the target path.\n"
                "Argument order is not important but --version for coral language version, "
                "--target as target path and atleast one .coral file must be passed "
                "directly or through directories containing one or more .coral file.",
            .targetFunction = command_compile
        });

    CommandManager::AddCommand(
        {
            .primaryParam = "version",
            .description = "Prints current Coral compiler version",
            .targetFunction = command_version
        });
}

int main(int argc, char* argv[])
{
    Core::Run(argc, argv, AddExternalCommands);

    return 0;
}