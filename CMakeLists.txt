cmake_minimum_required(VERSION 3.25)

project(EventBus LANGUAGES CXX)

# -----------------
# library interface
# ------------------------------------------------------------------

add_library(EventBus INTERFACE)

target_compile_features(EventBus INTERFACE cxx_std_20)

if(WIN32)
    target_compile_options(
        EventBus
        INTERFACE /EHs- /GR- /WAll /WX /permissive-
    )
else()
    target_compile_options(
        EventBus
        INTERFACE -fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic
    )
endif()

target_include_directories(
    EventBus
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ------------
# dummy target
# ------------------------------------------------------------------

add_executable(dummy EXCLUDE_FROM_ALL source/dummy.cpp)

target_link_libraries(dummy PRIVATE EventBus)
