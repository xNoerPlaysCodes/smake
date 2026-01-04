#ifndef Smake__project_config_hpp
#define Smake__project_config_hpp

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace smake {
    int read_project_config(std::string file);
    std::vector<std::string> generate_project_make(std::string file);
}

#endif//Smake__project_config_hpp
