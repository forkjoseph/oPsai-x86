LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= androidvncserver

LOCAL_SRC_FILES:= \
	LibVNCServer-0.9.7/libvncserver/main.c \
	LibVNCServer-0.9.7/libvncserver/rfbserver.c \
	LibVNCServer-0.9.7/libvncserver/rfbregion.c \
	LibVNCServer-0.9.7/libvncserver/auth.c \
	LibVNCServer-0.9.7/libvncserver/sockets.c \
	LibVNCServer-0.9.7/libvncserver/stats.c \
	LibVNCServer-0.9.7/libvncserver/corre.c \
	LibVNCServer-0.9.7/libvncserver/hextile.c \
	LibVNCServer-0.9.7/libvncserver/rre.c \
	LibVNCServer-0.9.7/libvncserver/translate.c \
	LibVNCServer-0.9.7/libvncserver/cutpaste.c \
	LibVNCServer-0.9.7/libvncserver/httpd.c \
	LibVNCServer-0.9.7/libvncserver/cursor.c \
	LibVNCServer-0.9.7/libvncserver/font.c \
	LibVNCServer-0.9.7/libvncserver/draw.c \
	LibVNCServer-0.9.7/libvncserver/selbox.c \
	LibVNCServer-0.9.7/libvncserver/d3des.c \
	LibVNCServer-0.9.7/libvncserver/vncauth.c \
	LibVNCServer-0.9.7/libvncserver/cargs.c \
	LibVNCServer-0.9.7/libvncserver/minilzo.c \
	LibVNCServer-0.9.7/libvncserver/ultra.c \
	LibVNCServer-0.9.7/libvncserver/scale.c \
	LibVNCServer-0.9.7/libvncserver/zlib.c \
	LibVNCServer-0.9.7/libvncserver/zrle.c \
	LibVNCServer-0.9.7/libvncserver/zrleoutstream.c \
	LibVNCServer-0.9.7/libvncserver/zrlepalettehelper.c \
	LibVNCServer-0.9.7/libvncserver/zywrletemplate.c \
	LibVNCServer-0.9.7/libvncserver/tight.c \
		fbvncserver.c 


LOCAL_LDLIBS +=  -llog -lz -ldl -landroid	
LOCAL_CFLAGS  +=  -Wall \
									-O3 \
									-DLIBVNCSERVER_HAVE_ZLIB \
									-DLIBVNCSERVER_HAVE_LIBPNG \
									-DLIBVNCSERVER_HAVE_LIBJPEG
									
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../screenMethods \
	$(LOCAL_PATH)/LibVNCServer-0.9.7/libvncserver \
	$(LOCAL_PATH)/LibVNCServer-0.9.7 \
	$(LOCAL_PATH)/LibVNCServer-0.9.7/libvncserver \
	$(LOCAL_PATH)/LibVNCServer-0.9.7/rfb \
	$(LOCAL_PATH)/../libpng \
	$(LOCAL_PATH)/../jpeg \
	$(LOCAL_PATH)/../jpeg-turbo \
	
LOCAL_STATIC_LIBRARIES := libjpeg libpng

include $(BUILD_EXECUTABLE)
