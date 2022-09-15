cmake_minimum_required (VERSION 3.1)
project (RCT2RideGen C)

find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${PNG_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ../libIsoRender/src)
link_directories(${PNG_LIBRARY_DIRS} ${ZLIB_LIBRARY_DIRS} ../libIsoRender)

add_executable(makevehicle src/main.c src/project.c)
set_property(TARGET makevehicle PROPERTY C_STANDARD 99)

#Use static libpng and zlib on Windows because the dynamic version doesn't work for some reason
IF (WIN32)
	target_link_libraries (makevehicle png.a z.a)
ELSE()
	target_link_libraries (makevehicle ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF()

target_link_libraries (makevehicle m zip assimp embree3 jansson IsoRender)

