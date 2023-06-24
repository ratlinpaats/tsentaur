#ifndef COMMONFUNCT_H
#define COMMONFUNCT_H

#include <winsock2.h>

const char* addrtostr(const sockaddr* addr);
wchar_t* ConvertToWideChar(const char* str);

wchar_t* GetUserName();

wchar_t* GetDomainName();

wchar_t* GetMachineName();

#endif  // COMMONFUNCT_H
