add_library(middlewares INTERFACE)

target_include_directories(middlewares INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include(FetchContent)

# --------------------
# Assimp
# --------------------
FetchContent_Declare(
  assimp
  URL https://github.com/assimp/assimp/archive/refs/tags/v5.4.3.zip
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

target_link_libraries(middlewares INTERFACE assimp::assimp)

# --------------------
# nlohmann/json
# --------------------
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip
)

set(JSON_Install OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(middlewares INTERFACE nlohmann_json::nlohmann_json)