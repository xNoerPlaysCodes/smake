#ifndef Smake__util_hpp
#define Smake__util_hpp

namespace util {
    struct global_state_cliargs_t {
        bool build = false;
        bool norebuild = false;
        bool genclangd = false;
    };
    void init_clistate(global_state_cliargs_t args);
    global_state_cliargs_t get_clistate();
}

#endif//Smake__util_hpp

