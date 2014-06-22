ifneq ($(TARGET_ARCH_ABI),armeabi-v7a)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	src/address.cpp \
	src/clock.cpp \
	src/ctx.cpp \
	src/dealer.cpp \
	src/decoder.cpp \
	src/devpoll.cpp \
	src/dist.cpp \
	src/encoder.cpp \
	src/epoll.cpp \
	src/err.cpp \
	src/fq.cpp \
	src/io_object.cpp \
	src/io_thread.cpp \
	src/ip.cpp \
	src/ipc_address.cpp \
	src/ipc_connecter.cpp \
	src/ipc_listener.cpp \
	src/kqueue.cpp \
	src/lb.cpp \
	src/mailbox.cpp \
	src/msg.cpp \
	src/mtrie.cpp \
	src/object.cpp \
	src/options.cpp \
	src/own.cpp \
	src/pair.cpp \
	src/pgm_receiver.cpp \
	src/pgm_sender.cpp \
	src/pgm_socket.cpp \
	src/pipe.cpp \
	src/poll.cpp \
	src/poller_base.cpp \
	src/precompiled.cpp \
	src/proxy.cpp \
	src/pub.cpp \
	src/pull.cpp \
	src/push.cpp \
	src/random.cpp \
	src/reaper.cpp \
	src/rep.cpp \
	src/req.cpp \
	src/router.cpp \
	src/select.cpp \
	src/session_base.cpp \
	src/signaler.cpp \
	src/socket_base.cpp \
	src/stream_engine.cpp \
	src/sub.cpp \
	src/tcp.cpp \
	src/tcp_address.cpp \
	src/tcp_connecter.cpp \
	src/tcp_listener.cpp \
	src/thread.cpp \
	src/trie.cpp \
	src/v1_decoder.cpp \
	src/v1_encoder.cpp \
	src/xpub.cpp \
	src/xsub.cpp \
	src/zmq.cpp \
	src/zmq_utils.cpp \
	src/ifaddrs.c

LOCAL_C_INCLUDES := /include \
					/src
LOCAL_CFLAGS += -DAVOID_TABLES 
LOCAL_CFLAGS += -Wall \
				-O3 

LOCAL_MODULE:= zmq

include $(BUILD_STATIC_LIBRARY)

endif
