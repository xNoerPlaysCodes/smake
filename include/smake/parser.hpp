#ifndef Smake__parser_hpp
#define Smake__parser_hpp

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
namespace smake {
    using variable_t = std::pair<std::string, std::string>;
    using variables_t = std::unordered_map<std::string, std::string>;

    enum class scope_t {
        task,
    };
    using scopes_t = std::unordered_map<scope_t, bool>;

    struct task_t {
        std::string name;
        std::vector<std::string> dependencies;
        std::vector<std::vector<std::string>> commands;
    };

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

    struct parser_context_t {
    private:
        bool finished = false;
        friend parser_context_t *parser_exec(std::vector<std::string> lines);
    public:
        variables_t vars;
        scopes_t scopes;
        std::vector<task_t> tasks;
        task_t cur_task;

        void register_variable(variable_t v) {
            if (finished) return;
            vars[v.first] = v.second;
            // std::cout << "Dbg: AddVariable(" << v.first << "," << v.second << ')' << '\n';
        }

        void register_task(task_t task) {
            if (finished) return;
            tasks.push_back(task);
            // std::cout << "Dbg: AddTask(\n";
            // std::cout << "        Name=" << task.name << '\n';
            // std::cout << "        Deps=[" << vecutil_join(task.dependencies, ",") << "]" << '\n';
            // std::cout << "        Cmds=[\n";
            // for (auto &cmd : task.commands) {
            //     std::cout << "           " << vecutil_join(cmd, " ") << '\n';
            // }
            // std::cout << "        ]\n";
            // // std::cout << "        Cmds=[" << vecutil_join(, ",") << "]" << '\n';
            // std::cout << ")\n";
        }

        std::string replace_str_with_vars(std::string str) {
            std::string s;
            bool brace_begin = false;
            std::string variable_name;
            bool exit = false;
            for (auto &c : str) {
                if (c == '{') {
                    brace_begin = true;
                    continue;
                }

                if (c == '}') {
                    brace_begin = false;
                    if (vars.find(variable_name) != vars.end()) {
                        s += vars[variable_name];
                    } else {
                        std::cerr << "undefined variable: " << variable_name << '\n';
                        exit = true;
                    }
                    variable_name = "";
                    continue;
                }

                if (brace_begin) {
                    variable_name += c;
                    continue;
                }

                s += c;
            }

            if (exit) std::exit(1);

            return s;
        }
    };

    parser_context_t *parser_exec(std::vector<std::string> lines);
}

#endif//Smake__parser_hpp
