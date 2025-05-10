#include "TCPClient.hpp"

namespace rodr
{
    namespace tcp
    {
        //creates and starts tcp connection to specified addres on given port
        TCPClient::TCPClient(const char* ip_addr, u_short port)
        {
            if (WSAStartup(MAKEWORD(2, 2), &data_) != 0)
            {
                std::cerr << "TCP: WSAStartup failed with error code: " << WSAGetLastError() << std::endl;
                return;
            }
            
            //socket creation
            socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (socket_ == INVALID_SOCKET)
            {
                std::cerr << "TCP: Socket creation failed with error code: " << WSAGetLastError() << std::endl;
                WSACleanup();
                return;
            }

            //setting socket receive timeout
            if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&RECV_MSG_TIMEOUT, sizeof(RECV_MSG_TIMEOUT)))
            {
                std::cerr << "TCP: Setsockopt failed with error code: " << WSAGetLastError() << std::endl;
                closesocket(socket_);
                WSACleanup();
                return;
            }

            //client setup
            client_.sin_family = AF_INET;
            int ret = inet_pton(AF_INET, ip_addr, &client_.sin_addr.S_un.S_addr);

            if (ret <= 0)
            {
                if (ret == 0) std::cerr << "TCP: Invalid client address format: " << ip_addr << std::endl; 
                else std::cerr << "TCP: inet_pton failed for client with error code: " << WSAGetLastError() << std::endl;

                WSACleanup();
                return;
            }
            client_.sin_port = htons(port);

            if (connect(socket_, (sockaddr*)&client_, sizeof(client_)) == SOCKET_ERROR)
            {
                std::cerr << "TCP: Connect failed with error code: " << WSAGetLastError() << std::endl;
                closesocket(socket_);
                WSACleanup();
                return;
            }
        }

        TCPClient::~TCPClient()
        {
            closesocket(socket_);
            WSACleanup();
        }

        //Tests if tcp connection works properly by pinging TCP server 3 times
        //Returns std::pair of whether all pings were succesfull and number of successful pings
        std::pair<bool, unsigned char> TCPClient::Test() const
        {
            unsigned char successful_pongs = 0;
            bool success = true;
            constexpr unsigned int buf_size = 16;

            char buffer[buf_size] = { 0 };

            for (int i = 0; i < 3; ++i)
            {   
                SendMsg("PING");
                ReceiveAndHandle(buffer, buf_size, [](const char*){return;}, [](const char*){return;});

                //if msg received is not pong
                if (strcmp(buffer, "PONG\r\n"))
                {
                    success = false;
                    continue;
                }
                successful_pongs++;
            }
            
            return std::make_pair(success, successful_pongs);
        }

        //sends given message
        void TCPClient::SendMsg(const char* msg) const
        {
            int sent_bytes = send(socket_, msg, strlen(msg), 0);
            if (sent_bytes == SOCKET_ERROR) std::cerr << "TCP: Send failed with error code: " << WSAGetLastError() << std::endl;
        }

        //recieves data from server and processes it
        void TCPClient::ReceiveAndHandle(rodr::handler handler_function, rodr::handler err_handler_function) const
        {
            char buffer[DEFAULT_BUFFER_SIZE] = { 0 };
            int received = recv(socket_, buffer, DEFAULT_BUFFER_SIZE, 0);

            if (received > 0) 
            {
                std::cout << "TCP: Received: " << buffer << std::endl;
                handler_function(buffer);
            }
            else if (received == 0) std::cout << "TCP: Connection closed." << std::endl;
            else
            {
                char last_err[8];
                snprintf(last_err, sizeof(last_err), "%d", WSAGetLastError());

                std::cerr << "TCP: Receive failed with error code: " << last_err << std::endl;
                err_handler_function(last_err);
            }
        }

        //recieves data from server on given buffer and processes it
        void TCPClient::ReceiveAndHandle(char* buffer, const unsigned int buffer_size, rodr::handler handler_function, rodr::handler err_handler_function) const
        {
            int received = recv(socket_, buffer, buffer_size, 0);

            if (received > 0) 
            {
                std::cout << "TCP: Received: " << buffer << std::endl;
                handler_function(buffer);
            }
            else if (received == 0) std::cout << "TCP: Connection closed." << std::endl;
            else
            {
                char last_err[8];
                snprintf(last_err, sizeof(last_err), "%d", WSAGetLastError());

                std::cerr << "TCP: Receive failed with error code: " << last_err << std::endl;
                err_handler_function(last_err);
            }
        }
    }
}
