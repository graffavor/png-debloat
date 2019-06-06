#include "cli_args.h"

using namespace std;

enum parser_states {
    CHECK_KEY = 0,
    NEED_VALUE
};

void ArgParser::parse(int argc, char** argv)
{
    std::string key;
    parser_states state = parser_states::CHECK_KEY;

    VArg* ref = nullptr;

    for (size_t i = 1; i < argc; i++) {
        switch (state) {
            case CHECK_KEY: {
                if (argv[i][0] == '-') {
                    key = string(argv[i]).substr(1);

                    for (auto r : args) {
                        if (key == r->get_name()) {
                            if (r->expects_value()) {
                                ref = r;
                                state = NEED_VALUE;
                            } else {
                                r->set_value("");
                                key.clear();
                            }
                            break;
                        }
                    }
                } else {
                    positional.emplace_back(argv[i]);
                }
                break;
            }
            case NEED_VALUE: {
                if (argv[i][0] == '-') {
                    throw missing_arg_error(key);
                } else if (ref != nullptr) {
                    ref->set_value(argv[i]);
                    ref = nullptr;
                    key.clear();
                    state = CHECK_KEY;
                };

                break;
            }
        }
    }

    if (!key.empty() && ref != nullptr) {
        throw missing_arg_error(key);
    }
}