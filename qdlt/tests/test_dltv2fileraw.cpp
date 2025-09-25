#include <gtest/gtest.h>

#include <dlt_common.h>


TEST(Dltv2_file_raw,parse_file){
    DltFile importfile;
    dlt_file_init(&importfile,0);
    auto result = dlt_file_open(&importfile,"testfile.dlt",0);
    EXPECT_EQ(result,0);
    auto version = (dlt_file_check_version(&importfile,0)&0xe0) >>5;
    EXPECT_EQ(version,2);
    auto parsed=(dltv2_file_read_raw(&importfile,0,0));
    EXPECT_EQ(parsed,1);
    EXPECT_EQ((&importfile)->msg.baseheaderv2.len,0x4700);//it is 0x0047 but due to endianess is stored as 0x4700
    EXPECT_EQ((&importfile)->msg.datasize,38);
    EXPECT_EQ((&importfile)->msg.extendedheaderv2.len,15);
    EXPECT_EQ((&importfile)->msg.optionalheaderv2.len,11);
}