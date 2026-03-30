cmake_minimum_required(VERSION 3.16)

project(domoticsController VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NANOGUI_ROOT "/home/ayrley/priv/nanogui" CACHE PATH "Path to NanoGUI build/install root")

# Keep source list dynamic like the Makefile's wildcard.
file(GLOB SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/Buttons/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/Pages/*.cpp"
)

add_executable(control ${SRCS})

target_compile_definitions(control
    PRIVATE
        NANOGUI_USE_OPENGL
        NANOGUI_SHARED
        NVG_SHARED
)

target_include_directories(control
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/Buttons
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/Pages
        ${NANOGUI_ROOT}/include
    ${NANOGUI_ROOT}/include/nanovg
    ${NANOGUI_ROOT}/ext/nanovg/src
)

set_target_properties(control PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(control PROPERTIES BUILD_RPATH "${NANOGUI_ROOT}")

find_package(Threads REQUIRED)
target_link_libraries(control PRIVATE Threads::Threads nanogui)
target_link_directories(control PRIVATE ${NANOGUI_ROOT})

if(DEFINED ENV{HOST_DIR})
    target_link_directories(control PRIVATE $ENV{HOST_DIR}/usr/lib)
endif()

if(DEFINED ENV{TARGET_DIR})
    target_link_directories(control PRIVATE $ENV{TARGET_DIR}/usr/lib)
endif()

add_custom_target(control-linter
    COMMAND clang-tidy ${SRCS} -- -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/src -I${CMAKE_CURRENT_SOURCE_DIR}/src/GUI -I${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/Buttons -I${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/Pages
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-tidy on domoticsController sources"
)