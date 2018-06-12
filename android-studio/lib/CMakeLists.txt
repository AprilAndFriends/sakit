cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(sakit)

include("../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(sakit STATIC ${CppSrc})

include_directories(AFTER "../../include/sakit")
include_directories(AFTER "../../src")
include_directories(AFTER "../../../hltypes/include")

add_definitions(
	-DSAKIT_EXPORTS
)

target_link_libraries(
	sakit
)
