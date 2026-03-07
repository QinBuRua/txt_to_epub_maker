cmake_minimum_required(VERSION 4.0)
project(txt_to_epub_maker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(includes INCLUDES_SRC)
aux_source_directory(utils/pugixml UTILS_PUGIXML_SRC)

add_library(CommonCore STATIC
        ${INCLUDES_SRC}
        ${UTILS_PUGIXML_SRC}
)
target_include_directories(CommonCore PUBLIC includes utils)

add_executable(AppMain
        src/main.cpp
)
target_link_libraries(AppMain PUBLIC CommonCore)

add_executable(AppTest
        src/test.cpp
)
target_link_libraries(AppTest PUBLIC CommonCore)