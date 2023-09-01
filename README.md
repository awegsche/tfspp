# TFS file reading library for C++

## Usage

The library is header only, so you should be fine by just cloning the repo / downloading the needed
header files.

If you're using cmake:

```cmake

FetchContent_Populate(
    tfspp
    QUIET
    GIT_REPOSITORY https://github.com/awegsche/tfspp.git
    GIT_TAG 0.1.0
    SOURCE_DIR tfspp
    )

add_subdirectory(${tfspp_SOURCE_DIR} ${tfspp_SOURCE_DIR}/build)
include_directories(${tfspp_SOURCE_DIR}/src)
```
