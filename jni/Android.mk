LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
#OPENCV_LIB_TYPE:=SHARED

#include C:\OpenCV-RC\opencv\sources\platforms\my_android\OpenCV.mk
#include C:\OpenCV3.1\opencv\sources\platforms\my_android\OpenCV.mk
#include C:\Users\Harsh\Downloads\OpenCV-3.1.0-android-sdk\OpenCV-android-sdk\sdk\native\jni\OpenCV.mk
#include C:\OpenCV3\opencv\sources\platforms\my_android\OpenCV.mk
#include C:\Users\Harsh\Downloads\OpenCV-3.0.0-rc1-android-sdk-1\OpenCV-android-sdk\sdk\native\jni\OpenCV.mk
include C:\Work\OpenCV4Android\OpenCV-3.0.0-android-sdk-1\OpenCV-android-sdk\sdk\native\jni\OpenCV.mk
#include C:\NVPACK\OpenCV-2.4.8.2-Tegra-sdk\sdk\native\jni\OpenCV.mk

LOCAL_SRC_FILES  := tracker.cpp objects.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_LDLIBS     += -llog -ldl
LOCAL_MODULE     := tracker

include $(BUILD_SHARED_LIBRARY)
