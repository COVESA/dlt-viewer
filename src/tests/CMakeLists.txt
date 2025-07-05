set(CMAKE_AUTOMOC ON)

qt_add_executable(test_dltfileindexer test_dltfileindexer.cpp)
add_test(NAME test_dltfileindexer COMMAND test_dltfileindexer)
target_link_libraries(test_dltfileindexer PRIVATE
    dlt_common
    qdlt
    Qt::Core
    Qt::Gui
    Qt::Test
    Qt::Widgets
)
