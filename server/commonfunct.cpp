#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>

#include "commonfunct.h"

const char* addrtostr(const sockaddr* addr)
{
    static char ipString[INET6_ADDRSTRLEN];

    if (addr->sa_family == AF_INET) {
        const sockaddr_in* ipv4 = reinterpret_cast<const sockaddr_in*>(addr);
        if (inet_ntop(AF_INET, &(ipv4->sin_addr), ipString, INET_ADDRSTRLEN) == NULL) {
            return "ERROR";
        }
    } else if (addr->sa_family == AF_INET6) {
        const sockaddr_in6* ipv6 = reinterpret_cast<const sockaddr_in6*>(addr);
        if (inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipString, INET6_ADDRSTRLEN) == NULL) {
            return "ERROR";
        }
    } else {
        return "ERROR";
    }

    return ipString;
}

wchar_t* ConvertToWideChar(const char* str)
{
    int length = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    wchar_t* wideStr = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, str, -1, wideStr, length);
    return wideStr;
}


wchar_t* GetUserName()
{

    char username[256];
    DWORD bufferSize = sizeof(username);

    GetUserNameA(username, &bufferSize);
    return ConvertToWideChar(username);
}

wchar_t* GetDomainName()
{
    char domain[16];
    DWORD bufferSize = sizeof(domain);
    GetComputerNameExA(ComputerNameDnsDomain, domain, &bufferSize);
    return ConvertToWideChar(domain);
}

wchar_t* GetMachineName()
{
    char computerName[16];
    DWORD bufferSize = sizeof(computerName);
    GetComputerNameA(computerName, &bufferSize);
    return ConvertToWideChar(computerName);
}