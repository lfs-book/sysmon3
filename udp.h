// udp.h
#ifndef sysmonUDP_H
#define sysmonUDP_H

#include <netinet/in.h>
#include <QHostInfo>

class sysmonUDP 
{
public:
    sysmonUDP( QString* server, int port = 12686 );
    ~sysmonUDP(){};
    QString getData();

private:
    bool lookupOK;
    int  udp_socket;

    struct sockaddr_in server_socket; 
};
#endif 
