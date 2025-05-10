#pragma once
#include <string>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>

#include "rodrUtils.hpp"

#pragma comment(lib, "ws2_32.lib")

//if on vsc add "-lws2_32" to tasks.json/args
namespace rodr
{   
    namespace tcp
    {
        constexpr unsigned int DEFAULT_BUFFER_SIZE = 64; 
        constexpr unsigned int RECV_MSG_TIMEOUT = 5000;//ms

        class TCPClient
        {
        public:
            TCPClient(const char* ip_addr, u_short port);
            ~TCPClient();

            std::pair<bool, unsigned char> Test() const;
            void SendMsg(const char* msg) const;
            void ReceiveAndHandle(rodr::handler handler_function, rodr::handler err_handler_function) const;
            void ReceiveAndHandle(char* buffer, const unsigned int buffer_size, rodr::handler handler_function, rodr::handler err_handler_function) const;

        private:
            SOCKET socket_ = INVALID_SOCKET;
            sockaddr_in client_;
            WSAData data_;
        };
    }
}
