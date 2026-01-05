#include "smake/exec.hpp"
#include "smake/parser.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <smake/macros.hpp>
#include <smake/project_config.hpp>
#include <util.hpp>

void set_cli_arguments(int argc, char *argv[]);
static std::string vecutil_join(
    const std::vector<std::string>& vec,
    std::string_view delim,
    size_t start = 0
) {
    if (start >= vec.size())
        return {};

    std::string s;
    for (size_t i = start; i < vec.size(); ++i) {
        if (!s.empty())
            s += delim;
        s += vec[i];
    }
    return s;
}


int main(int argc, char **argv) {
    set_cli_arguments(argc, argv);

    auto clistate = util::get_clistate();

    if (clistate.build) {
        std::string file = "smakecfg.yml";

        bool genbuild_smake_success = false;

        if (std::filesystem::exists("build.smake") && (!std::filesystem::exists("smakecfg.yml"))) {
            std::cout << "smake: entering command-runner only mode\n";
            genbuild_smake_success = true;
            goto parser;
        }

        smake::read_project_config(file);

        {
            std::vector<std::string> lines = smake::generate_project_make(file);
            if (lines.at(0) != "__skip_write__" || clistate.norebuild) {
                std::ofstream f("build.smake");
                if (!f.is_open()) {
                    std::cerr << "smake: unable to create file: " << "build.smake" << '\n';
                    return 1;
                }
                for (auto &l : lines) {
                    f << l << '\n';
                }
                f.flush();
                f.close();
            }
            genbuild_smake_success = true;
        }

parser: {
            smake::parser_context_t *ctx = nullptr;

            if (genbuild_smake_success) {
                std::ifstream f("build.smake");
                std::string line;
                std::vector<std::string> lines;
                while (std::getline(f, line)) {
                    lines.push_back(line);
                }
                f.close();

                ctx = smake::parser_exec(lines);
            }

            if (ctx == nullptr) {
                std::cerr << "cannot continue\n";
                std::exit(1);
            }

            std::vector<std::string> tasks_executed;

            for (auto &task : ctx->tasks) {
                bool task_continue = false;
                for (auto &dep : task.dependencies) {
                    if (std::find(tasks_executed.begin(), tasks_executed.end(), dep) == tasks_executed.end()) {
                        task_continue = true;
                    }
                }
                if (task_continue) continue;
                bool all_cmds_successful = true;
                for (auto &cmd : task.commands) {
                    if (cmd.empty()) continue;

                    std::string argv0 = cmd.at(0);
                    cmd.erase(cmd.begin());

                    int ret = smake::run_command(argv0, cmd);
                    if (ret != 0) {
                        std::cout << "FAILED: " << argv0 << ' ' << vecutil_join(cmd, " ") << '\n';
                        all_cmds_successful = false;
                    }
                }
                if (all_cmds_successful)
                    tasks_executed.push_back(task.name);
            }

            if (tasks_executed.size() == ctx->tasks.size()) {
                std::cout << "smake: done\n";
                return 0;
            } else {
                std::cout << "smake: subcommand(s) failed\n";
                return 1;
            }
        }
    }
}

void set_cli_arguments(int argc, char *argv[]) {
    if (argc < 0) {
        return;
    }

    const std::vector<std::string> args_with_values = {
    };
    util::global_state_cliargs_t args = {};
    bool exit = false;
    bool error = false;
    std::string additional_exit_message;
    for (int i = 1; i < argc; ++i) {
        int nexti = i + 1;
        std::string arg = argv[i];
        std::string value = "";
        if (nexti != argc) {
            if (argv[nexti][0] != '-' && argv[nexti][0] != '/') {
                value = argv[nexti];
            }
        }

        std::string rawarg = arg;

        bool argument_detected = false;
        bool too_many_prefixes;

        if (arg.starts_with("--")) {
            arg = arg.substr(2);
            too_many_prefixes = argument_detected;
            argument_detected = true;
        }
        if (arg.starts_with("-")) {
            arg = arg.substr(1);
            too_many_prefixes = argument_detected;
            argument_detected = true;
        }
        if (arg.starts_with("/")) {
            arg = arg.substr(1);
            too_many_prefixes = argument_detected;
            argument_detected = true;
        }

        if (too_many_prefixes) {
            exit = true;
            error = true;
            std::cerr << "unexpected argument: " << rawarg << '\n';
        }

        if (value.empty() && std::find(args_with_values.begin(), args_with_values.end(), arg) != args_with_values.end()) {
            std::cerr << "argument " << arg << " does not have a value where it is required" << '\n';
            continue;
        }

        if (!value.empty()) {
            ++i;
        }

        if (arg == "version" || arg == "v") {
            exit = true;

            std::vector<std::string> lines = {
                "Smake (Smay-ke) " + std::string(SMAKE__VERSION),
                "Copyright (C) 2026 noerlol",
                "License MIT: You are free to change and redistribute",
                "             but must keep this license",
                "",
                "Written by noerlol",
            };

            for (auto &l : lines) {
                std::cout << l << '\n';
            }
        } else if (arg == "build" || arg == "b") {
            args.build = true;
        } else if (arg == "no-rebuild") {
            args.norebuild = true;
        } else if (arg == "gen-clangd") {
            args.genclangd = true;
        }

        else if (arg == "help") {
            exit = true;

            std::vector<std::string> lines = {
                "Usage: " + std::string(argv[0]) + " [options]",
                "",
                "Arguments may start with java-style (-), GNU-long-style (--) or windows-style (/) on any platform\n",
                "Arguments:",
                "   version, v",
                "   -> returns version",
                "",
                "   build, b",
                "   -> compile the current project",
                "",
                "   no-rebuild",
                "   -> skips rebuilding build.smake (advanced)",
                "",
                "   gen-clangd",
                "   -> generates .clangd files for includes (advanced)",
                "",
                "Values to arguments marked with * are mandatory",
            };

            for (auto &line : lines) {
                std::cout << line << '\n';
            }
        }
    }

    if (error && exit) {
        std::cerr << "one or more errors found in parser\n";
        if (!additional_exit_message.empty()) {
            std::cerr << additional_exit_message << '\n';
        }
        std::exit(1);
    }

    if (exit) {
        std::exit(1);
    }

    util::init_clistate(args);
}
