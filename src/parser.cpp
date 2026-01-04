#include <cstddef>
#include <filesystem>
#include <iostream>
#include <smake/parser.hpp>
#include <smake/macros.hpp>

namespace smake {
    std::vector<std::string> strutil_split(std::string str, char delim) {
        std::vector<std::string> out;
        std::string current;

        for (char c : str) {
            if (c == delim) {
                out.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        out.push_back(current); // last part

        return out;
    }

    void strutil_trim(std::string& s) {
        s.erase(
            s.begin(),
            std::find_if(s.begin(), s.end(),
                [](unsigned char ch) { return !std::isspace(ch); })
        );

        s.erase(
            std::find_if(s.rbegin(), s.rend(),
                [](unsigned char ch) { return !std::isspace(ch); }).base(),
            s.end()
        );
    }

    std::string strutil_drop(std::string str, char delim) {
        bool str_begin = false;
        bool esc_begin = false;
        std::string s;
        for (int i = 0; i < str.size(); ++i) {
            if (str.at(i) == '"') {
                if (!esc_begin)
                    str_begin = !str_begin;
                else
                    esc_begin = false;
            }

            if (str.at(i) == '\\') {
                esc_begin = true;
            }

            if (!str_begin && str.at(i) == delim) {
                break;
            }

            s += str.at(i);
        }
        return s;
    }

#define CONTINUE { ++ln; continue; }
#define LINE(n) "smparser: " " " "[" << n << "] "
#define KW(kw) "kw '" << kw << "' "

    // root
    parser_context_t *parser_exec(std::vector<std::string> lines) {
        parser_context_t *ctx = new parser_context_t;

        ctx->register_variable({ "PROJECT_DIRECTORY", std::filesystem::absolute(std::filesystem::current_path() / "test").string() });
        ctx->register_variable({ "SMAKE_VERSION", SMAKE__VERSION });

        int ln = 1;
        for (auto &line : lines) {
            strutil_trim(line);
            if (line.empty()) CONTINUE;
            if (line.starts_with("#")) CONTINUE;
            line = strutil_drop(line, '#');
            std::vector<std::string> args = strutil_split(line, ' ');
            size_t hd = 0;
            std::string kw = args.at(hd);
            ++hd;

            if (kw == "var") {
                if (args.size() < 3) {
                    std::cerr << LINE(ln) << KW(kw) "takes only 3 arguments\n";
                    CONTINUE;
                }

                std::string var_name = args.at(hd);

                std::string joined = vecutil_join(args, " ", hd + 1);
                int i = 0;
                bool str_begin = false;
                bool esc_begin = false;
                std::string s;
                for (auto &c : joined) {
                    if (c == '"') {
                        if (!esc_begin) {
                            str_begin = !str_begin;
                        } else {
                            esc_begin = false;
                        }
                    }

                    if (str_begin && c != '\\' && c != '"') {
                        s += c;
                    }

                    if (c == '\\') {
                        esc_begin = true;
                    }

                    i++;
                }

                ctx->register_variable({ var_name, ctx->replace_str_with_vars(s) });
            } else if (kw == "task.s") {
                if (ctx->scopes[scope_t::task]) {
                    std::cerr << LINE(ln) << "nested " << KW(kw) << "is not supported\n";
                    CONTINUE;
                }
                ctx->scopes[scope_t::task] = true;
                std::string name = args[hd];
                ctx->cur_task.name = name;

                bool dep_begin = false;
                for (int i = (hd + 1); i < args.size(); ++i) {
                    std::string s = args.at(i);

                    if (dep_begin) {
                        dep_begin = false;
                        ctx->cur_task.dependencies.push_back(s);
                    }

                    if (s == "-d") {
                        dep_begin = true;
                    }
                }
            } else if (kw == "task.e") {
                if (!ctx->scopes[scope_t::task]) {
                    std::cerr << LINE(ln) << KW(kw) << "unexpected, scope was never opened\n";
                    CONTINUE;
                }
                ctx->scopes[scope_t::task] = false;
                ctx->register_task(ctx->cur_task);
                ctx->cur_task = {};
            } else if (kw == "exec") {
                if (!ctx->scopes[scope_t::task]) {
                    std::cerr << LINE(ln) << KW(kw) << "may only be used in " << KW("task.s") "scope\n";
                    CONTINUE;
                }

                ctx->cur_task.commands.push_back({});

                // exec ls -l -v
                for (int i = hd; i < args.size(); ++i) {
                    std::string p = ctx->replace_str_with_vars(args.at(i));
                    ctx->cur_task.commands.at(ctx->cur_task.commands.size() - 1).push_back(p);
                }
            }
            else {
                std::cerr << LINE(ln) << "unknown keyword: " << kw << '\n';
            }

            ++ln;
        }

        ctx->finished = true;

        return ctx;
    }
}
