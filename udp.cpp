#include <QHostInfo>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "udp.h"

sysmonUDP::sysmonUDP( QString* server, int port ) 
{
    QHostInfo hostInfo;
    QHostInfo returnedHost = hostInfo.fromName( *server );

    if ( returnedHost.error() != QHostInfo::NoError )
    {
       lookupOK = false;
       return;
    }

    lookupOK = true;

    QString ip = returnedHost.addresses().first().toString();

//qDebug() << "Found address:" << ip.toLatin1();     

     struct timeval tv;
     tv.tv_sec  = 1;
     tv.tv_usec = 0;

     // server socket
     memset( (char*) &server_socket, 0, sizeof(server_socket) );
     server_socket.sin_family      = AF_INET;
     server_socket.sin_port        = htons( port );
     server_socket.sin_addr.s_addr = inet_addr( ip.toLatin1() );

     udp_socket = socket( AF_INET, SOCK_DGRAM, 0);

     setsockopt( udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) );
}

QString sysmonUDP::getData()
{
    if ( ! lookupOK ) return "Bad IP lookup";

    QByteArray data_out = QByteArray( "Send data" );

//qDebug() << "Size of data_out should be 9: " << data_out.size();

    // Ask for data
    ssize_t i =
       sendto( udp_socket,
               data_out.data(),
               data_out.size() + 1, // Add trailing null
               MSG_CONFIRM,
               (struct sockaddr*)& server_socket,
               sizeof( server_socket ) );

//qDebug() << "Bytes sent: " << i;

    if ( i < 0 ) return "UDP out failed";

//qDebug() << "ip:" << ip << ";  port:" << serverPort; // ok

    struct sockaddr_in client_socket;

    memset( (char*)& client_socket, 0, sizeof(client_socket) );
    client_socket.sin_family      = AF_INET;
    client_socket.sin_port        = htons( 0 );
    client_socket.sin_addr.s_addr = htonl( INADDR_ANY );

    socklen_t socket_len;
    #define BUFFER_SIZE 1024
    char data_in [ BUFFER_SIZE ];
    memset( (char*)& data_in, 0, BUFFER_SIZE );

    // Now read the response
    int n = 
       recvfrom( udp_socket,
                 data_in,
                 BUFFER_SIZE,
                 MSG_WAITALL,
                 (struct sockaddr*)& client_socket,
                 &socket_len );
//qDebug() << "Bytes received: " << n;
//qDebug() << data_in;
    if ( n < 0 )
       return "Timeout";
 
    //data_in[ n ] = '\0'; // make sure the string is properly terminated
    QString s = QString( data_in );
//qDebug() << s << "\n--- size of string:" << s.size();
    //return QString( data_in );
    return s;
}
