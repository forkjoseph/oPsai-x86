#include <android/log.h>

#define LOG_TAG "Zmqserver"
#define L(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

void getIP(char *ip);