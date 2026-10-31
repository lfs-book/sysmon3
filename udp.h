// udp.h
#ifndef sysmonUDP_H
#define sysmonUDP_H

#include <QString>
#include <time.h>
#include <unistd.h>     // close

class sysmonUDP 
{
public:
    sysmonUDP( QString* server, int port = 12686 );
    ~sysmonUDP() { if ( udp_socket >= 0 ) close( udp_socket ); }

    // Copying would close the same socket twice
    // We don't anticipate needing these, but they prevent copying
    sysmonUDP           ( const sysmonUDP& ) = delete;
    sysmonUDP& operator=( const sysmonUDP& ) = delete;

    QString getData();

private:
    bool    open( void );   // Look up the server and connect the socket

    QString host;
    int     port;
    int     udp_socket = -1;
    time_t  lastTry    = 0; // Time of the last failed open()
};
#endif 
