# TFS file reading library for C++

## Getting Started

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

## Usage

```cpp
#include <iostream>
#include "tfs_dataframe.h"

using TfsDataFrame = tfs::dataframe<double>;

int main(int argc, char** argc) {
    TfsDataFrame twiss{};

    std::vector double_column = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<std::string> string_column = {"one", "two", "three", "four", "five"};
    std::vector<int> int_column = {1, 2, 3, 4, 5};

    twiss.add_column(double_column, "doubles");
    twiss.add_column(string_column, "strings");
    twiss.add_column(int_column, "ints");

    twiss.insert_property("Q1", 62.31);
    twiss.insert_property("Q2", 60.32);
    twiss.insert_property("Comment", std::string{"hello world"});


    twiss.to_file("test_tfs.tfs");

    TfsDataFrame twiss_read{"test_tfs.tfs"};

    std::cout << twiss_read.pretty_print() << std::endl;

    return 0;
}

```
