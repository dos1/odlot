cmake_minimum_required(VERSION 2.8)

project(odlot C)

SET(LIBSUPERDERPY_GAMENAME "odlot" CACHE INTERNAL "")
SET(LIBSUPERDERPY_GAMENAME_PRETTY "ODLOT" CACHE INTERNAL "")

SET(LIBSUPERDERPY_APPID "com.holypangolin.ODLOT")
SET(LIBSUPERDERPY_ORIENTATION "LANSCAPE")
SET(LIBSUPERDERPY_VERSION "1.0")

set(EMSCRIPTEN_TOTAL_MEMORY "1024" CACHE INTERNAL "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/libsuperderpy/cmake")

include(libsuperderpy)

add_subdirectory(libsuperderpy)
add_subdirectory(src)
add_subdirectory(data)
