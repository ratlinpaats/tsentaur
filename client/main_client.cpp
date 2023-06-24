#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAXDATASIZE 2048
#define PORT "40454" // The port to which the client will be connecting

int main(int argc,char *argv[])
{
   struct addrinfo *p, hints, *res;
   int addrlen, nbytes, status;
   char buf[MAXDATASIZE], ac_server[INET6_ADDRSTRLEN];
   SOCKET sockfd;
   WSADATA s_wsaData;


   if (argc != 2)
   {
      fprintf(stderr,"usage: client hostname\n");

      return 0;
   }

   status = WSAStartup(MAKEWORD(2,2),&s_wsaData);

   if (status != 0)
   {
      fprintf(stderr,"WSAStartup failed with code %d.\n",status);

      return 0;
   }


   if (LOBYTE(s_wsaData.wVersion) < 2 ||
       HIBYTE(s_wsaData.wVersion) < 2)
   {
      fprintf(stderr,"Version 2.2 of Winsock is not available.\n");
      WSACleanup();

      return 0;
   }


   memset(&hints,0,sizeof(hints));

   hints.ai_family   = AF_UNSPEC;   // AF_INET or AF_INET6 to force version
   hints.ai_socktype = SOCK_STREAM; // Streaming socket


   if (getaddrinfo(argv[1],PORT,&hints,&res) != 0)
   {

      fprintf(stderr,"getaddrinfo catastrophically failed");
      WSACleanup();

      return 0;
   }

   for (p = res ; p != NULL ; p = p->ai_next)
   {
      sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
      if (sockfd == INVALID_SOCKET)
      {
         fprintf(stderr,"socket failed");
         continue;
      }

      addrlen = (int)p->ai_addrlen;
      status = connect(sockfd,p->ai_addr,addrlen);

      if (status == SOCKET_ERROR)
      {
         fprintf(stderr,"connect failed");
         closesocket(sockfd);
         continue;
      }

      break;
   }
/*                                                                            */
/* Check for a connection:                                                    */
/*                                                                            */
   if (p == NULL)
   {
      fprintf(stderr,"Failed to connect.\n");
      freeaddrinfo(p);
      WSACleanup();
      return 0;
   }
/*                                                                            */
/* Tell user to which server a connection was made:                           */
/*                                                                            */


   freeaddrinfo(res);

   nbytes = recv(sockfd,buf,MAXDATASIZE-1,0);

   if (nbytes == SOCKET_ERROR)
   {
      fprintf(stderr,"recv catastrophically failed");
      closesocket(sockfd);
      WSACleanup();

      return 0;
   }
   else if (nbytes == 0)
   {
      fprintf(stderr,"Socket closed by server.\n");
      closesocket(sockfd);
      WSACleanup();

      return 0;
   }


   buf[nbytes] = '\0';

   printf("Received '%s'\n",buf);

   closesocket(sockfd);

   WSACleanup();

   return 0;
}

