cmake_minimum_required (VERSION 3.1)
project (RCT2RideGen C)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR} ../libRCT2/src ../libIsoRender/src ../libImage/src /usr/local/Cellar/jansson/2.12/include)
link_directories(../libIsoRender ../libImage ../libRCT2 /usr/local/Cellar/jansson/2.12/lib/)
link_directories(/usr/local/Cellar/embree/3.6.1/lib)
link_directories(/usr/local/Cellar/assimp/5.0.0/lib)
link_directories(/usr/local/Cellar/embree/3.6.1/lib)
link_directories(/usr/local/Cellar/assimp/5.0.0/lib)

add_executable(makeride src/main.c src/project.c)
set_property(TARGET makeride PROPERTY C_STANDARD 99)
target_link_libraries (makeride ${PNG_LIBRARY} Image RCT2 IsoRender assimp embree3 jansson)

