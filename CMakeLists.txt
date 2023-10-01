cmake_minimum_required(VERSION 3.23)
project(LimitedSpace)

set(CMAKE_CXX_STANDARD 17)

add_executable(LimitedSpace
        main.cpp
        src/Game.cpp
        src/EntityManager.cpp
        src/Util.cpp
        src/System.cpp
        src/Loader.cpp
        src/SoundManager.cpp)

# -- Folder with headers
target_include_directories(LimitedSpace PRIVATE src)

# -- Add vendor dependencies
add_subdirectory("vendor/raylib")
add_subdirectory("vendor/entt")
add_subdirectory("vendor/nlohmann_json")

# -- Link everything statically into the exe
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static")

# -- Link statically with vendor libs
target_link_libraries(LimitedSpace PUBLIC
        raylib
        EnTT
        nlohmann_json)

# -- Copy assets into build folder
add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets)

add_dependencies(LimitedSpace copy_assets)

# -- Change output executable's name
set_target_properties(LimitedSpace PROPERTIES
        OUTPUT_NAME "Astralinda")

# -- Remove terminal window
set(CMAKE_EXE_LINKER_FLAGS "-mwindows")