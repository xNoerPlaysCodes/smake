#include <util.hpp>

namespace util {
    global_state_cliargs_t clistate;

    void init_clistate(global_state_cliargs_t args) {
        clistate = args;
    }
    global_state_cliargs_t get_clistate() {
        return clistate;
    }
}
