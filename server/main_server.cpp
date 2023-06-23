#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#define PORT "40454"
#define BACKLOG 20
#define MAXIMUM_CLIENTS 255
#define CHAR_LENGTH 32

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <windows.h>
#include <thread>

void HandleUserInput(char* input)
{
    char userInput[CHAR_LENGTH];
    while(true)
    {
        scanf("%s",userInput);
        if(userInput)
        {
        
        strcpy(input,userInput);
        
        }
    };
}

void HandleSocketPolling(WSAPOLLFD *pfds, int pfds_len, char* input)
{

    int num_events = 0;
    int addrlen, bytes_received;
    SOCKET listener, tempfd;
    struct sockaddr_storage clientaddr;
    char ac_buf[512];
    listener = pfds[0].fd;
    

    while(true)
    {
        printf("Polling...\n");

        printf("%s",input);
        memset(input, '\0', sizeof(input));
        

        num_events = WSAPoll(pfds, pfds_len, 3000);

        if (num_events == SOCKET_ERROR)
        {
            fprintf(stderr,"catastrophic failure when polling clients\n");
            closesocket(listener);
            WSACleanup();
        }
        else
        {
            for(int i = 0; i<pfds_len; i++)
            {
                if(pfds[i].revents & POLLIN || pfds[i].revents & POLLHUP)
                    {
                        if (pfds[i].fd == listener)
                        {
                            addrlen = (int)sizeof(clientaddr);
                            tempfd = accept (listener, (struct sockaddr *)&clientaddr, &addrlen);
                            if(tempfd == INVALID_SOCKET)
                            {
                                fprintf(stderr,"failed to accept a client");
                            }
                            else
                            {
                                if ((pfds_len) == (MAXIMUM_CLIENTS-1))
                                {
                                fprintf(stderr,"too many clients!");
                                }                                                                    
                            pfds[pfds_len].fd = tempfd;
                            pfds[pfds_len].events = POLLIN;
                            pfds[pfds_len].revents = 0;
                            pfds_len++;

                            printf("new connection %s on socket %lld\n",clientaddr.ss_family,tempfd);
                            }
                        }
                        else
                        {
                            bytes_received = recv(pfds[i].fd,ac_buf,sizeof(ac_buf),0);
                            tempfd = pfds[i].fd;
                            if (bytes_received == SOCKET_ERROR || bytes_received == 0)
                            {
                                if (bytes_received == 0) // Connection closed
                                {
                                    fprintf(stderr,"socket %lld hung up\n",tempfd);
                                }
                                else
                                {

                                    fprintf(stderr,"failed to receive data\n");

                                }
                                closesocket(pfds[i].fd);
                                pfds[i] = pfds[pfds_len-1];
                                pfds[pfds_len-1].fd = -1;
                                pfds[pfds_len-1].events = 0;
                                pfds[pfds_len-1].revents = 0;
                                pfds_len--;
                            }                                
                            else
                            {
                                ///here is where the message is received, cool
                                printf(ac_buf);
                            }
                        } 
                        
                    }

            }

        }
    }
}


int main()
{
    WSADATA s_wsaData;
    int status, num_events, addrlen, pfd_len;
    struct addrinfo hints, *res, *p; 
    
    SOCKET listener, tempfd;

    WSAPOLLFD pfds[MAXIMUM_CLIENTS];
    bool yes;
    char in_put[CHAR_LENGTH];


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


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res) !=0 )
    {
        fprintf(stderr,"can't get address info");
        WSACleanup();
        return 0;
    }

    for(p = res; p != NULL; p->ai_next)
    {
      listener = socket(p->ai_family,p->ai_socktype, p->ai_protocol);
      if (listener == INVALID_SOCKET)
      {
         fprintf(stderr,"socket catastrophically failed\n");
         continue;
      }
      yes = TRUE;
      status = setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(yes));
      if (status == SOCKET_ERROR)
      {
         fprintf(stderr,"setsockopt failed, probably an occupied port");
         closesocket(listener);
         freeaddrinfo(p);
         WSACleanup();
         return 0;
      }

      addrlen = (int)p->ai_addrlen;
      status = bind(listener,p->ai_addr,addrlen);

      if (status == SOCKET_ERROR)
      {

         fprintf(stderr,"bind catastrophically failed");
         closesocket(listener);
         continue;
      }
      break;
    };
    
    if (p == NULL)
    {
      fprintf(stderr,"bind failed, no valid sockets\n");
      closesocket(listener);
      freeaddrinfo(res);
      WSACleanup();

      return 0;
    }

    if (listen(listener, BACKLOG) == SOCKET_ERROR)
    {
        fprintf(stderr,"listen catastrophically failed");
        closesocket(listener);
        freeaddrinfo(res);
        WSACleanup();
    }

    freeaddrinfo(res);

    printf("Successfully initialized the server, polling clients...\n");
    pfds[0].fd=listener;
    pfds[0].events=POLLIN;
    pfds[0].revents=0;
    pfd_len = 1;
    
    std::thread userInputThread(HandleUserInput, in_put);

    // Start the socket polling thread
    std::thread socketPollingThread(HandleSocketPolling, pfds, pfd_len, in_put);

    // Wait for the threads to finish
    userInputThread.join();
    socketPollingThread.join();

    system("pause");

};