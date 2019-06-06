#ifndef PNG_DEBLOAT_LOGGING_H
#define PNG_DEBLOAT_LOGGING_H

#ifndef NO_VERBOSE_LOGS
#include <iostream>
#define LOG(level) std::cerr << "[" << #level << "]: "
#else
#include <fstream>
std::ofstream _noop_logger;
#define LOG(level) _noop_logger
#endif

#endif //PNG_DEBLOAT_LOGGING_H
