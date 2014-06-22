LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= opsaizmqserver

LOCAL_SRC_FILES:= \
	zmqserver.c \
	zmq3.2.4/src/address.cpp \
	zmq3.2.4/src/clock.cpp \
	zmq3.2.4/src/ctx.cpp \
	zmq3.2.4/src/dealer.cpp \
	zmq3.2.4/src/decoder.cpp \
	zmq3.2.4/src/devpoll.cpp \
	zmq3.2.4/src/dist.cpp \
	zmq3.2.4/src/encoder.cpp \
	zmq3.2.4/src/epoll.cpp \
	zmq3.2.4/src/err.cpp \
	zmq3.2.4/src/fq.cpp \
	zmq3.2.4/src/io_object.cpp \
	zmq3.2.4/src/io_thread.cpp \
	zmq3.2.4/src/ip.cpp \
	zmq3.2.4/src/ipc_address.cpp \
	zmq3.2.4/src/ipc_connecter.cpp \
	zmq3.2.4/src/ipc_listener.cpp \
	zmq3.2.4/src/kqueue.cpp \
	zmq3.2.4/src/lb.cpp \
	zmq3.2.4/src/mailbox.cpp \
	zmq3.2.4/src/msg.cpp \
	zmq3.2.4/src/mtrie.cpp \
	zmq3.2.4/src/object.cpp \
	zmq3.2.4/src/options.cpp \
	zmq3.2.4/src/own.cpp \
	zmq3.2.4/src/pair.cpp \
	zmq3.2.4/src/pgm_receiver.cpp \
	zmq3.2.4/src/pgm_sender.cpp \
	zmq3.2.4/src/pgm_socket.cpp \
	zmq3.2.4/src/pipe.cpp \
	zmq3.2.4/src/poll.cpp \
	zmq3.2.4/src/poller_base.cpp \
	zmq3.2.4/src/precompiled.cpp \
	zmq3.2.4/src/proxy.cpp \
	zmq3.2.4/src/pub.cpp \
	zmq3.2.4/src/pull.cpp \
	zmq3.2.4/src/push.cpp \
	zmq3.2.4/src/random.cpp \
	zmq3.2.4/src/reaper.cpp \
	zmq3.2.4/src/rep.cpp \
	zmq3.2.4/src/req.cpp \
	zmq3.2.4/src/router.cpp \
	zmq3.2.4/src/select.cpp \
	zmq3.2.4/src/session_base.cpp \
	zmq3.2.4/src/signaler.cpp \
	zmq3.2.4/src/socket_base.cpp \
	zmq3.2.4/src/stream_engine.cpp \
	zmq3.2.4/src/sub.cpp \
	zmq3.2.4/src/tcp.cpp \
	zmq3.2.4/src/tcp_address.cpp \
	zmq3.2.4/src/tcp_connecter.cpp \
	zmq3.2.4/src/tcp_listener.cpp \
	zmq3.2.4/src/thread.cpp \
	zmq3.2.4/src/trie.cpp \
	zmq3.2.4/src/v1_decoder.cpp \
	zmq3.2.4/src/v1_encoder.cpp \
	zmq3.2.4/src/xpub.cpp \
	zmq3.2.4/src/xsub.cpp \
	zmq3.2.4/src/zmq.cpp \
	zmq3.2.4/src/zmq_utils.cpp \
	zmq3.2.4/src/ifaddrs.c

LOCAL_LDLIBS +=  -llog -lz -ldl -landroid	
LOCAL_CFLAGS  +=  -Wall \
		-O3 
									
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/zmq3.2.4/include \
	$(LOCAL_PATH)/zmq3.2.4/src
	
LOCAL_STATIC_LIBRARIES := libzmq

include $(BUILD_EXECUTABLE)
