#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <objidl.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <gdiplus.h>

#include <vector>

#define MAXDATASIZE 2048
#define PORT "40454" // The port to which the client will be connecting


std::vector<BYTE> CaptureScreen()
{
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Get the screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a bitmap for the screen image
    HDC screenDC = GetDC(nullptr);
    HDC memoryDC = CreateCompatibleDC(screenDC);
    HBITMAP screenBitmap = CreateCompatibleBitmap(screenDC, screenWidth, screenHeight);
    HGDIOBJ oldBitmap = SelectObject(memoryDC, screenBitmap);

    // Copy the screen content onto the bitmap
    BitBlt(memoryDC, 0, 0, screenWidth, screenHeight, screenDC, 0, 0, SRCCOPY);

    // Create a Graphics object from the bitmap
    Gdiplus::Bitmap gdiBitmap(screenBitmap, nullptr);

    // Lock the bitmap for direct access to the pixel data
    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, screenWidth, screenHeight);
    gdiBitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);

    // Calculate the size of the raw pixel data
    size_t imageSize = bitmapData.Height * bitmapData.Stride;

    // Create a vector to hold the raw pixel data
    std::vector<BYTE> imageData(imageSize);

    // Copy the pixel data to the vector
    memcpy(imageData.data(), bitmapData.Scan0, imageSize);

    
    // Unlock the bitmap
    gdiBitmap.UnlockBits(&bitmapData);

    
    // Cleanup resources
    SelectObject(memoryDC, oldBitmap);
    DeleteObject(screenBitmap);
    DeleteDC(memoryDC);
    ReleaseDC(nullptr, screenDC);
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return imageData;
}


void SendScreenshot(SOCKET sockfd, std::vector<BYTE> imageData)
{

    int totalBytesSent = 0;
    int bytesRemaining = imageData.size();

    while (bytesRemaining > 0)
    {
        int bytesSent = send(sockfd, reinterpret_cast<char*>(imageData.data()) + totalBytesSent, bytesRemaining, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            fprintf(stderr, "failed to send the screenshot\n" );
            break;
        }

        totalBytesSent += bytesSent;
        bytesRemaining -= bytesSent;
    }
}

SOCKET ConnectToServer(char *servName)
{
   SOCKET sockfd;
   struct addrinfo *p, hints, *res;
   int addrlen, status;
   memset(&hints,0,sizeof(hints));

   hints.ai_family   = AF_UNSPEC;   // AF_INET or AF_INET6 to force version
   hints.ai_socktype = SOCK_STREAM; // Streaming socket

   while(true)
   {
      if (getaddrinfo(servName,PORT,&hints,&res) != 0)
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
            fprintf(stderr,"Socket failed, retrying\n");
            continue;
         }

         addrlen = (int)p->ai_addrlen;
         status = connect(sockfd,p->ai_addr,addrlen);

         if (status == SOCKET_ERROR)
         {
            fprintf(stderr,"Socket connect failed, retrying\n");
            closesocket(sockfd);
            continue;
         }

         break;
      }

      if (p == NULL)
      {
         fprintf(stderr,"Failed to connect, retrying.\n");
         freeaddrinfo(p);
      }
      else
      {
         break;
      }
   }

   printf("Successfully connected to %s.\n", servName);
   freeaddrinfo(res);
   return sockfd;
}

int main(int argc,char *argv[])
{

   int nbytes, status;
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


   
   sockfd = ConnectToServer(argv[1]);



   while(true)
   {
      nbytes = recv(sockfd,buf,MAXDATASIZE-1,0);
      if (nbytes == SOCKET_ERROR)
      {
         fprintf(stderr,"recv catastrophically failed or server unexpectedly shutdown.\n");
         sockfd = ConnectToServer(argv[1]);
         continue;
      }
      else if (nbytes == 0)
      {
         fprintf(stderr,"Socket closed by server.\n");
         sockfd = ConnectToServer(argv[1]);
         continue;
      }

      


      buf[nbytes] = '\0';

      printf("Received '%s'\n",buf);


   }

   closesocket(sockfd);

   WSACleanup();

   return 0;
}

