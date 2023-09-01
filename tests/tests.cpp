#include <gtest/gtest.h>
#include "../src/tfs_dataframe.h"

using TfsDataFrame = tfs::dataframe<double>;

TEST(ReadAndWriteTest, BasicAssertions) {
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

    ASSERT_EQ(double_column, twiss_read.get_column("doubles").as_real_vector());
    ASSERT_EQ(string_column, twiss_read.get_column("strings").as_string_vector());
    ASSERT_EQ(int_column, twiss_read.get_column("ints").as_int_vector());

    ASSERT_EQ(twiss.get_property("Q1").get_double(), 62.31);
    ASSERT_EQ(twiss.get_property("Q2").get_double(), 60.32);
    ASSERT_EQ(twiss.get_property("Comment").get_string(), std::string{"hello world"});
}
