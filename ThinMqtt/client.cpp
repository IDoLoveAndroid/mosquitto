#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "common.h"

#define CLIENT_IP       "127.0.0.1"
#define CLIENT_PORT     1884

#define CLIENT_ID       "SN0123456789"

static int g_fd;

static volatile sig_atomic_t g_interrupt = false;

static void sig_int_handler(int sig)
{
    g_interrupt = true;
}

static void wait_For_sig_int(void)
{
    while(g_interrupt == false) {
        ::usleep(100000);
    }
}

void thin_mqtt_connect(int fd)
{
    char buf[128] = {0};

    int bytes = ::snprintf(buf, 128, ">>%s,con,", CLIENT_ID);
    uint8_t cs = 0x00;
    for(int i = 0; i < bytes; i++) {
        cs ^= buf[i];
    }
    bytes += ::snprintf(buf + bytes, 128, "%02x\n", cs);

    ::write(fd, buf, bytes);
    LOGD("write: %s\n", buf);
}

void thin_mqtt_disconnect(int fd)
{
    char buf[128] = {0};

    int bytes = ::snprintf(buf, 128, ">>%s,discon,", CLIENT_ID);
    uint8_t cs = 0x00;
    for(int i = 0; i < bytes; i++) {
        cs ^= buf[i];
    }
    bytes += ::snprintf(buf + bytes, 128, "%02x\n", cs);

    ::write(fd, buf, bytes);
    LOGD("write: %s\n", buf);
}

void thin_mqtt_ping(int fd)
{
    char buf[128] = {0};

    int bytes = ::snprintf(buf, 128, ">>%s,ping,", CLIENT_ID);
    uint8_t cs = 0x00;
    for(int i = 0; i < bytes; i++) {
        cs ^= buf[i];
    }
    bytes += ::snprintf(buf + bytes, 128, "%02x\n", cs);

    ::write(fd, buf, bytes);
    LOGD("write: %s\n", buf);
}

void thin_mqtt_publish(int fd, const char *topic, const char *message)
{
    char buf[1024] = {0};

    int bytes = ::snprintf(buf, 1024, ">>%s,pub,%s,%s,", CLIENT_ID, topic, message);
    uint8_t cs = 0x00;
    for(int i = 0; i < bytes; i++) {
        cs ^= buf[i];
    }
    bytes += ::snprintf(buf + bytes, 1024, "%02x\n", cs);

    ::write(fd, buf, bytes);
    LOGD("write: %s\n", buf);
}

void *clientThread(void *ptr)
{
    while(!g_interrupt) {
        ::usleep(10000000);
        if(g_fd > 0) {
            thin_mqtt_ping(g_fd);
        }
    }
    LOGD("EXIT THREAD\n");
    return NULL;
}

int main(void)
{
    int sockfd;
    int len;
    struct sockaddr_in addr;
    int rc;
    pthread_t client_pid;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    addr.sin_port = htons(CLIENT_PORT);

    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        LOGE("Failed to create socket fd\n");
        return -1;
    }

    rc = ::connect(sockfd, (struct sockaddr *)&addr,
            sizeof(struct sockaddr));
    if(rc < 0) {
        LOGE("Failed to connect\n");
        ::close(sockfd);
        return -1;
    }
    g_fd = sockfd;
    rc = pthread_create(&client_pid, NULL, clientThread, NULL);
    if(rc < 0) {
        LOGE("%s: failed to create client thread\n", __func__);
        return -1;
    }
    thin_mqtt_connect(sockfd);
    ::usleep(4000000);
    thin_mqtt_publish(sockfd, "VU/SN0123456789/ENGINE/RPM", "2000");

    wait_For_sig_int();
    g_fd = -1;
    thin_mqtt_disconnect(sockfd);
    ::close(sockfd);

    return 0;
}
