#pragma once

#define CLOSESOCK(s) if(INVALID_SOCKET != s){closesocket(s); s=INVALID_SOCKET;}

using OnError = void(*)(const SOCKET s);
using OnReadComplete = void(*)(const SOCKET s, char* data, int len);
using OnWriteComplete = void(*)(size_t bytes);

