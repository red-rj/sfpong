cmake_minimum_required(VERSION 3.15)

if(MSVC)
  add_compile_options(-Zc:__cplusplus)
endif()

set(CPPFILES  config.cpp convert.cpp game.cpp joyinput.cpp main.cpp menu.cpp)
set(HEADERS  ci_string.h common.h convert.h game_config.h game.h gvar.h 
             imgui_inc.h imgui_scoped.h joyinput.h menu.h rng.h)

add_executable(sfpong ${CPPFILES})

find_package(SFML 2.6 CONFIG REQUIRED COMPONENTS graphics system)
find_package(ImGui-SFML REQUIRED)
find_package(Boost 1.75 COMPONENTS property_tree REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(lyra CONFIG REQUIRED)
find_package(Catch2)

target_compile_options(sfpong PRIVATE cxx_std_20)

target_link_libraries(sfpong PRIVATE 
    # SFML::Graphics SFML::Window 
    sfml-system sfml-graphics
    ImGui-SFML::ImGui-SFML 
    Boost::property_tree
    fmt::fmt 
    spdlog::spdlog
    bfg::lyra
)
