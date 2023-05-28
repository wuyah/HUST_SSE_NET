cmake_minimum_required(VERSION 3.0.0)
project(Net_2 VERSION 0.1.0)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB XDB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/external/*.c)

add_library(xdb STATIC ${XDB_FILES})

add_executable(Net_2 ${SRC_FILES})

set(NPCAP_INCLUDE_DIR "D:/WebTool/npcap-sdk-1.13/Include")
set(NPCAP_LIBRARY "D:/WebTool/npcap-sdk-1.13/Lib/x64/wpcap.lib")
set(RAPID_JSON_INCLUDE_DIR "D:\\WebTool\\rapidjson-master\\rapidjson-master\\include")

include_directories(${NPCAP_INCLUDE_DIR})
include_directories(${RAPID_JSON_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${NPCAP_LIBRARY})
target_link_libraries(${PROJECT_NAME} Ws2_32.lib)
target_link_libraries(${PROJECT_NAME} wsock32)
target_link_libraries(${PROJECT_NAME} xdb)