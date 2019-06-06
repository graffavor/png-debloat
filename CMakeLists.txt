cmake_minimum_required(VERSION 3.2)

project(png_debloat VERSION 1.4)

option(WITHOUT_PYTHON "Do not build python libraries" 0)
option(WITHOUT_BINARY "Do not build binary files" 0)

option(SUPPRESS_LOGS "Disable all debug and info logging messages" 0)

# Python options
option(PY_EXT_VERSION "Version info for python extension" ${PROJECT_VERSION})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES include/cli_args.h include/pngparser.h src/pngparser.cpp include/crc.h src/cli_args.cpp include/logging.h)

find_package(PNG)

if (${PNG_FOUND})
    add_definitions(-DWITH_LIBPNG)
    add_definitions(${PNG_DEFINITIONS})
endif()

if (${SUPPRESS_LOGS})
    add_definitions(-DNO_VERBOSE_LOGS)
endif()

if (NOT ${WITHOUT_BINARY})
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} src/main.cpp)

    target_include_directories(${PROJECT_NAME} PRIVATE include)
    target_include_directories(${PROJECT_NAME} PRIVATE src)

    if (${PNG_FOUND})
        target_include_directories(${PROJECT_NAME} PRIVATE ${PNG_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARY})
    endif()
endif()

if (NOT ${WITHOUT_PYTHON})
    find_package(PythonInterp 3.4 REQUIRED)

    add_subdirectory(pybind11)

    set(PYBIND11_CPP_STANDARD -std=c++11)
    set(PY_MODULE_NAME _png_debloat)

    pybind11_add_module(${PY_MODULE_NAME} ${SOURCE_FILES} py/bindings.cpp)
    add_definitions(-DVERSION_INFO="${PY_EXT_VERSION}")

    target_include_directories(${PY_MODULE_NAME} PRIVATE include)
    target_include_directories(${PY_MODULE_NAME} PRIVATE src)

    if (${PNG_FOUND})
        target_include_directories(${PY_MODULE_NAME} PRIVATE ${PNG_INCLUDE_DIRS})
    endif()
endif()
