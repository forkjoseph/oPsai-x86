
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE:= screenshot

	
LOCAL_LDLIBS +=  -llog -lz -ldl -landroid
LOCAL_CFLAGS  +=  -Wall \
									-O3 
LOCAL_SRC_FILES := \
	fb2png.c \
	img_process.c \
	screenshot-main.c \
	fb.c 

LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/../libpng \
		$(LOCAL_PATH)/../jpeg \
		$(LOCAL_PATH)/../jpeg-turbo \
		
LOCAL_STATIC_LIBRARIES := libjpeg libpng 

include $(BUILD_EXECUTABLE)

