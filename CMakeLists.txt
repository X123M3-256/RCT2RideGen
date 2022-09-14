cmake_minimum_required (VERSION 3.1)
project (RCT2RideGen C)

find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)


include_directories(${PNG_INCLUDE_DIR} ../libIsoRender/src)
link_directories(../libIsoRender)

add_executable(makevehicle src/main.c src/project.c)
set_property(TARGET makevehicle PROPERTY C_STANDARD 99)
target_link_libraries (makevehicle m png.a z.a IsoRender assimp embree3 jansson)

