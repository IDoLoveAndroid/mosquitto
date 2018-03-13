#ifndef __THIN_MQTT_CLIENT_H__
#define __THIN_MQTT_CLIENT_H__

#include <pthread.h>
#include <vector>
#include <mosquittopp.h>

#include "common.h"

namespace iot {

class ThinMqtt {
public:
    ThinMqtt();
    virtual ~ThinMqtt();

    static void *mainThread(void *ptr);
    static void *clientThread(void *ptr);
    bool setup(const char *addr, int port);
    void release();

    void on_message(const struct mosquitto_message *message);
private:
    static void process(ThinMQTTClient *client);
    int makeNoneBlock(int fd);
    ThinMQTTClient *getClient(int fd);

private:
    pthread_t mMainThread;
    std::vector<ThinMQTTClient *> mClients;
    int mPort;
    char mHost[64];
    int mPipeFds[2];
};
};

#endif
