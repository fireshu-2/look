cmake_minimum_required(VERSION 3.0.0)

project(ssne_ai_demo)

include(cmake_config/Paths.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2")
add_compile_options(-Wno-psabi)

add_executable(ssne_ai_demo ${PROJECT_SOURCE_DIR}/demo_face_detection.cpp)

# ================== 头文件路径 ==================
include_directories(${M1_SDK_INC_DIR})
include_directories(${M1_SDK_INC_DIR}/smartsoc)
include_directories(${CMAKE_SOURCE_DIR}/include)

# 如果third_party有额外头文件
if(EXISTS "${SDK_ROOT}/third_party/include")
    include_directories(${SDK_ROOT}/third_party/include)
endif()

# ================== 源文件 ==================
set(SRC_DIR "src")
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/*.cpp")
target_sources(ssne_ai_demo PRIVATE ${SRC_FILES})

# ================== 库搜索路径 ==================
link_directories(
    ${M1_SDK_LIB_DIR}
)

if(EXISTS "${SDK_ROOT}/third_party/lib")
    link_directories(${SDK_ROOT}/third_party/lib)
endif()

# ================== 链接库 ==================
target_link_libraries(${PROJECT_NAME}
    ${M1_SSNE_LIB}
    ${M1_CMABUFFER_LIB}
    ${M1_OSD_LIB}
    ${M1_SSZLOG_LIB}
    ${M1_ZLOG_LIB}
    ${M1_EMB_LIB}
    pthread
    dl
)

install(TARGETS ssne_ai_demo DESTINATION bin)