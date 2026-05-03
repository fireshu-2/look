# Paths.cmake for SSNE AI Demo
# ==============================

# SDK根目录 - m1_sdk_lib 构建输出
# 在Docker容器内的路径
set(SDK_ROOT "/home/smartsens_flying_chip_a1_sdk/A1_SDK_SC235HAI/smartsens_sdk/output/build/m1_sdk_lib/usr" CACHE PATH "SDK root directory")

# ================== 路径设置 ==================

# include目录 (包含 smartsoc/ 子目录)
set(M1_SDK_INC_DIR "${SDK_ROOT}/include" CACHE PATH "M1 SDK include directory")

# lib目录
set(M1_SDK_LIB_DIR "${SDK_ROOT}/lib" CACHE PATH "M1 SDK library directory")

# ================== 库文件设置 ==================
# 使用完整路径确保链接器能找到

set(M1_SSNE_LIB        "${M1_SDK_LIB_DIR}/libssne.so"        CACHE STRING INTERNAL)
set(M1_CMABUFFER_LIB   "${M1_SDK_LIB_DIR}/libcmabuffer.so"   CACHE STRING INTERNAL)
set(M1_OSD_LIB         "${M1_SDK_LIB_DIR}/libosd.so"         CACHE STRING INTERNAL)
set(M1_SSZLOG_LIB     "${M1_SDK_LIB_DIR}/libsszlog.so"      CACHE STRING INTERNAL)
set(M1_ZLOG_LIB       "${M1_SDK_LIB_DIR}/libzlog.so"        CACHE STRING INTERNAL)
set(M1_EMB_LIB        "${M1_SDK_LIB_DIR}/libemb.so"         CACHE STRING INTERNAL)

# ================== 调试信息 ==================
message(STATUS "========================================")
message(STATUS "SSNE AI Demo CMake Configuration")
message(STATUS "========================================")
message(STATUS "SDK_ROOT: ${SDK_ROOT}")
message(STATUS "M1_SDK_INC_DIR: ${M1_SDK_INC_DIR}")
message(STATUS "M1_SDK_LIB_DIR: ${M1_SDK_LIB_DIR}")
message(STATUS "M1_SSNE_LIB: ${M1_SSNE_LIB}")
message(STATUS "========================================")