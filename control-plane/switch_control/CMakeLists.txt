# Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code

cmake_minimum_required(VERSION 3.5.1)

project(tofino_switch_control C CXX)

set(CMAKE_CXX_STANDARD 17)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

find_package(Threads REQUIRED)
link_libraries(pthread)


# Protobuf & GRPC

include_directories($ENV{SDE_INSTALL}/include/)
link_directories($SDE/install/lib)

include_directories($ENV{SDE_INSTALL}/include/bf_switchd)
link_directories($ENV{SDE_INSTALL}/include/bf_switchd) 

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")


# loguru
include_directories(/opt/loguru/)
link_libraries(dl) # library needed for luguru
set(LIB_SOURCES ${LIB_SOURCES} /opt/loguru/loguru.cpp)

#Boost
find_package(Boost 1.67.0 REQUIRED )
link_libraries( ${Boost_LIBRARIES} )


# BF SDE related stuff 

set(SDE_LIB_PATH $ENV{SDE_INSTALL}/lib)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include(GNUInstallDirs)

include_directories($ENV{SDE_INSTALL}/include/)

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)


# Tofino switchd libraries
find_library(PYTHON_LIBRARY libpython3.so PATHS ${SDE_LIB_PATH})
find_library(AVAGO_LIBRARY libavago.so PATHS ${SDE_LIB_PATH})
find_library(TOFINO_PLATFORM_MANAGER_LIBRARY libpltfm_mgr.so PATHS ${SDE_LIB_PATH})
find_library(TOFINO_PLATFORM_MANAGER_THRIFT_LIBRARY libpltfm_mgr_thrift.so PATHS ${SDE_LIB_PATH})
find_library(BF_SWITCHD_LIBRARY libbf_switch.so PATHS ${SDE_LIB_PATH})
find_library(DRIVER_LIBRARY libdriver.so PATHS ${SDE_LIB_PATH})
find_library(BFSYS_LIBRARY libbfsys.so PATHS ${SDE_LIB_PATH})
find_library(BFUTILS_LIBRARY libbfutils.so PATHS ${SDE_LIB_PATH})
find_library(BF_SHELL_PLUGIN_CLISH bfshell_plugin_clish.so PATHS ${SDE_LIB_PATH})
find_library(BF_SHELL_PLUGIN_PIPEMGR bfshell_plugin_pipemgr.so PATHS ${SDE_LIB_PATH})
find_library(BF_SHELL_PLUGIN_DEBUG bfshell_plugin_debug.so PATHS ${SDE_LIB_PATH})
find_library(BF_SHELL_PLUGIN_BFRT bfshell_plugin_bf_rt.so PATHS ${SDE_LIB_PATH})

link_libraries(
    ${PYTHON_LIBRARY} ${AVAGO_LIBRARY} ${DRIVER_LIBRARY} ${BFSYS_LIBRARY} ${BFUTILS_LIBRARY} ${TOFINO_PLATFORM_MANAGER_LIBRARY} ${TOFINO_PLATFORM_MANAGER_THRIFT_LIBRARY} ${BF_SWITCHD_LIBRARY}
    ${BF_SHELL_PLUGIN_CLISH} ${BF_SHELL_PLUGIN_PIPEMGR} ${BF_SHELL_PLUGIN_DEBUG} ${BF_SHELL_PLUGIN_BFRT}
)
link_libraries(stdc++)
link_libraries(m)

# executable

file(GLOB SRCS
    "*.cpp"
    "tables/*.cpp"
    "flow_classification/*.cpp"
)

include_directories(.)

add_executable(tofino_switch_control ${SRCS} ${LIB_SOURCES})

set_target_properties(tofino_switch_control PROPERTIES LINK_WHAT_YOU_USE "TRUE")

target_link_libraries(tofino_switch_control Threads::Threads gmp gmpxx ${Boost_LIBRARIES} dl)

#print_all_variables()
