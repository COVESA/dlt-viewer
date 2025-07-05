#include "dlt_common.h"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#define DLT_DAEMON_TEXTSIZE 10024

// TEST(ParseDltFile, initialize_open_free) {
//     DltFile file;
//     static char text[DLT_DAEMON_TEXTSIZE];
//     /* Get PWD so file can be used*/
//     std::filesystem::path openfile{std::filesystem::current_path() / "testfile_10.dlt"};
//     std::filesystem::path output{"/tmp/output_testfile_10.txt"};
//
//     /* Normal Use-Case, expected 0 */
//     EXPECT_LE(0, dlt_file_init(&file, 0));
//     EXPECT_LE(0, dlt_file_open(&file, openfile.c_str(), 0));
//
//     while (dlt_file_read(&file, 0) >= 0) {
//         // throw std::exception{};
//     }
//
//     for (int i = 0; i < file.counter; i++) {
//         EXPECT_LE(0, dlt_file_message(&file, i, 0));
//         EXPECT_LE(0, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 0));
//     }
//
//     // for (int i = 0; i < file.counter; i++) {
//     //     EXPECT_LE(0, dlt_file_message(&file, i, 0));
//     //     EXPECT_LE(0, dlt_message_print_ascii(&file.msg, text, DLT_DAEMON_TEXTSIZE, 1));
//     // }
//
//     EXPECT_LE(0, dlt_file_free(&file, 0));
// }

TEST(ParseDltFile, memory_mapped_file) {
    using namespace boost::interprocess;

    // Define file names
    const char* FileName = "testfile_10.dlt";

    // Open the file mapping and map it as read-only
    boost::interprocess::file_mapping m_file(FileName, read_only);

    mapped_region region(m_file, read_only);

    // Get the address of the mapped region
    void* addr = region.get_address();
    std::size_t size = region.get_size();

    // Check that memory was initialized to 1
    const char* mem = static_cast<char*>(addr);
    for (std::size_t i = 0; i < size; ++i)
        if (*mem++ != 1)
            std::cout << *mem;
}

// TEST(ParseDltFile, filebuffer) {
//     // Define file names
//     const char* FileName = "testfile_10.dlt";
//     const std::size_t FileSize = 10000;
//
//     // Now test it reading the file
//     std::filebuf fbuf;
//     fbuf.open(FileName, std::ios_base::in | std::ios_base::binary);
//
//     // Read it to memory
//     std::vector<char> vect(FileSize, 0);
//     fbuf.sgetn(&vect[0], std::streamsize(vect.size()));
//
//     // Check that memory was initialized to 1
//     const char* mem = static_cast<char*>(&vect[0]);
//     for (std::size_t i = 0; i < FileSize; ++i)
//         if (*mem++ != 1)
//             std::cout << *mem;
// }
