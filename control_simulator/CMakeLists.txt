cmake_minimum_required(VERSION 3.11.0)

project(EasyStream_OrinDetection VERSION 1.0.0 LANGUAGES CUDA CXX C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lX11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CXX_STD "14" CACHE STRING "C++ standard")

# 查找 OpenCV 库
find_package(OpenCV REQUIRED)

# 添加动态链接库
set(dynamic_libs 
  pthread 
  pcap
)

if(NOT WIN32)
ADD_DEFINITIONS("-std=c++11")
endif(NOT WIN32)

include_directories(${struct2x_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories("/usr/local/include")
link_directories("/usr/local/lib")
FILE(GLOB_RECURSE sourcefiles "src/*.cpp")



add_executable(terminal terminal.cpp ${PROJECT_SOURCE_DIR} ${sourcefiles} ${VPS_PCAP})
target_link_libraries(terminal ${dynamic_libs} -pthread ${OpenCV_LIBS} )

add_executable(multicast_recv multicast_recv.cpp ${PROJECT_SOURCE_DIR} ${sourcefiles} ${VPS_PCAP})
target_link_libraries(multicast_recv ${dynamic_libs} -pthread ${OpenCV_LIBS} )
