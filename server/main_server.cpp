#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <windows.h>


int main()
{
   WSADATA                   s_wsaData;
   int                       i_status;


/*                                                                            */
/* Initialize Winsock and request version 2.2:                                */
/*                       
                                                     */
   i_status = WSAStartup(MAKEWORD(2,2),&s_wsaData);


   if (i_status != 0)
   {
      fprintf(stderr,"WSAStartup failed with code %d.\n",i_status);
      return 1;
   }

/*                                                                            */
/* Verify that version 2.2 is available:                                      */
/*                                                                            */
   if (LOBYTE(s_wsaData.wVersion) < 2 ||
       HIBYTE(s_wsaData.wVersion) < 2)
   {
      fprintf(stderr,"Version 2.2 of Winsock is not available.\n");
      WSACleanup();
      return 2;
   }


};