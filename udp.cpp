#include <QHostInfo>
#include <QHostAddress>

#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "udp.h"

// sysmond ignores requests shorter than its largest possible reply
// (DATA_SIZE in sysmond.h, currently 1024 bytes), so the request is
// padded to that size.  The reply is at most DATA_SIZE bytes too.
static const int SYSMOND_DATA_SIZE = 1024;


sysmonUDP::sysmonUDP( QString* server, int serverPort )
   : host( *server ), port( serverPort )
{
    open();   // If this fails, getData() tries again later
}

// Look up the server and create a socket connected to it.
//
// A connected UDP socket only receives datagrams from the server, and
// reports "connection refused" right away when nothing is listening on
// the server's port, instead of waiting for the timeout.
bool sysmonUDP::open()
{
   // The lookup blocks, and with the network down it can take several
   // seconds, so after a failure don't try again for 10 seconds.
   time_t now = time( nullptr );
   if ( lastTry != 0 && now - lastTry < 10 ) return false;
   lastTry = now;
 
   QHostInfo info = QHostInfo::fromName( host );
   if ( info.error() != QHostInfo::NoError ) return false;
 
   // The socket is IPv4 only, so use the first IPv4 address.  The lookup
   // may also return IPv6 addresses, e.g. ::1 for localhost.
   for ( const QHostAddress& address : info.addresses() )
   {
      if ( address.protocol() != QAbstractSocket::IPv4Protocol ) continue;
 
      int sock = socket( AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0 );
      if ( sock < 0 ) return false;
 
      // server socket
      struct sockaddr_in server_socket;
      memset( &server_socket, 0, sizeof( server_socket ) );
      server_socket.sin_family      = AF_INET;
      server_socket.sin_port        = htons( port );
      server_socket.sin_addr.s_addr = htonl( address.toIPv4Address() );
 
      if ( ::connect( sock, (struct sockaddr*) &server_socket,
                      sizeof( server_socket ) ) < 0 )
      {
         close( sock );
         return false;
      }
      // Wait at most one second for a reply
      struct timeval tv = { 1, 0 };
      setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) );
 
      udp_socket = sock;
      lastTry    = 0;
      return true;
   }
 
   return false;   // No IPv4 address
}

QString sysmonUDP::getData()
{
   if ( udp_socket < 0 && ! open() ) return "Bad IP lookup";

   // Throw away anything already waiting: a reply that arrived after an
   // earlier request timed out.  Otherwise this request would get that
   // old reply, and every later request the one before it.  This also
   // clears a pending "connection refused" from an earlier request.
   char discard[ 16 ];
   while ( recv( udp_socket, discard, sizeof( discard ), MSG_DONTWAIT ) >= 0 );

   // Ask for data.  The content is ignored; the rest is zeros.
   static const char request[ SYSMOND_DATA_SIZE ] = "Send data";

   if ( send( udp_socket, request, sizeof( request ), 0 ) < 0 )
      return "UDP out failed";

   // Read the reply.  One extra byte so the string is always terminated.
   char    reply[ SYSMOND_DATA_SIZE + 1 ];
   ssize_t n = recv( udp_socket, reply, SYSMOND_DATA_SIZE, 0 );

   if ( n < 0 )
      return ( errno == ECONNREFUSED ) ? "Connection refused" : "Timeout";

   // sysmond ends the reply with a NUL, but don't depend on it
   reply[ n ] = '\0';

   return QString::fromUtf8( reply );
}
