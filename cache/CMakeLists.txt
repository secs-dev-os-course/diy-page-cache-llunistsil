cmake_minimum_required(VERSION 3.12)
project(cache
        VERSION 0.0.0
        DESCRIPTION "cache module"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_compile_options(/utf-8)

add_library(cache SHARED
        cache_api.h
        cache_api.cpp
        cache.h
        cache.cpp
)

target_include_directories(cache PUBLIC ${CMAKE_SOURCE_DIR})

add_definitions(-CACHE_EXPORTS)
target_compile_definitions(cache PRIVATE
        CACHE_EXPORTS
)
