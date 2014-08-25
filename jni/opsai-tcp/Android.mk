LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= opsaitcpserver

LOCAL_SRC_FILES:= \
	tcpserver.c \
	ifaddrs.c
LOCAL_LDLIBS +=  -llog -lz -ldl -landroid	
LOCAL_CFLAGS  +=  -Wall \
		-O3 
									
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \

include $(BUILD_EXECUTABLE)