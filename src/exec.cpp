#include <smake/exec.hpp>
#include <string>
#include <vector>

#ifdef _WIN32
#define __platform_run_command(root, args) __win32_run_command(root, args)
#endif

#if defined(__linux__) || defined(__linux) || defined(__APPLE__) || defined(__MACH__)
#define __platform_run_command(root, args) __unix_run_command(root, args)
#endif

#if defined(__linux__) || defined(__linux) || defined(__APPLE__) || defined(__MACH__)
#include <unistd.h>
#include <sys/wait.h>
int __unix_run_command(std::string root, std::vector<std::string> &args) {
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> argv;
        argv.reserve(args.size() + 2);

        argv.push_back(const_cast<char*>(root.c_str())); // argv[0]

        for (const auto& s : args)
            argv.push_back(const_cast<char*>(s.c_str()));

        argv.push_back(nullptr);

        execvp(root.c_str(), argv.data());

        _exit(1); // exec failed
    } else {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }
}
#endif

#ifdef _WIN32
int __win32_run_command(std::string, std::vector<std::string>) {
    std::cout << "windows not supported!\n";
}
#endif

namespace smake {
    int run_command(std::string root, std::vector<std::string> &args) {
        return __platform_run_command(root, args);
    }
}
