add_executable(test_qdlt
    test_dltmessagematcher.cpp
    test_dltctrlmsg.cpp
    test_qdltargument.cpp
    test_qdltmsgwrapper.cpp
)
target_link_libraries(
  test_qdlt
  PRIVATE
    GTest::gtest_main
    qdlt
)

add_test(
  NAME test_qdlt
  COMMAND $<TARGET_FILE:test_qdlt>
)

# opt manager has its own test executable since its a singleton
add_executable(test_dltoptmanager
    test_dltoptmanager.cpp
)

target_link_libraries(
  test_dltoptmanager
  PRIVATE
    GTest::gtest_main
    qdlt
)

add_test(
  NAME test_dltoptmanager
  COMMAND $<TARGET_FILE:test_dltoptmanager>
)

# dltv2fileraw test configures its working directory to use test data file located in the source dir
add_test(
  NAME test_dltv2fileraw
  COMMAND $<TARGET_FILE:test_dltv2fileraw> 
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_executable(test_dltv2fileraw
    test_dltv2fileraw.cpp
)
target_link_libraries(
  test_dltv2fileraw
  PRIVATE
    GTest::gtest_main
    qdlt
)
