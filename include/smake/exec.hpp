#ifndef Smake__exec_hpp
#define Smake__exec_hpp

#include <string>
#include <vector>

namespace smake {
    int run_command(std::string root, std::vector<std::string> &args);
}

#endif//Smake__exec_hpp
