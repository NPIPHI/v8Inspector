cmake_minimum_required(VERSION 3.4.1)

project(v8Inspector)

add_library(v8Inspector STATIC
        include/v8Inspector/Inspector.h
        src/Inspector.cpp
        src/utils.h
        src/v8inspector_channel.cpp src/v8inspector_channel.h
        src/v8inspector_client.cpp src/v8inspector_client.h
        )

add_dependencies(v8Inspector AndriodWebsocket)

target_link_libraries(v8Inspector PUBLIC AndriodWebsocket)

if(DEFINED V8_INCLUDE_DIR)
    target_include_directories(v8Inspector PUBLIC ${V8_INCLUDE_DIR})
else()
    message(FATAL_ERROR "V8_INCLUDE_DIR not set")
endif()

target_include_directories(v8Inspector PUBLIC include)