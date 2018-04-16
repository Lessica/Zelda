//
// Created by Zheng on 05/03/2018.
//

#import <string>
#import <list>
#import <arpa/inet.h>

#import "ZeldaLogger.h"
#import "ZeldaHTTPRequest.h"
#import "ZeldaHTTPResponse.h"
#import "ZeldaAuthenticationAgent.h"

#define ZELDA_MODE_PLAIN "plain"
#define ZELDA_MODE_TUNNEL "tunnel"
#define ZELDA_MODE_TCP "tcp"

typedef std::string ZeldaMode;

class Zelda {

    /* Public Methods */

public:

#pragma mark - Initializers

    Zelda(const std::string &address, int port);

#pragma mark - Address Options

    std::string GetAddress();
    int GetPort();

    std::string GetRemoteAddress();
    void SetRemoteAddress(const std::string &address);
    int GetRemotePort();
    void SetRemotePort(int port);

#pragma mark - Advanced Options

    bool GetUseSplice();
    void SetUseSplice(bool splice);

    int GetMaxConnection();
    void SetMaxConnection(int count);

#pragma mark - Loggers

    ZeldaLogger *GetLogger();
    void SetLogger(ZeldaLogger *logger);

#pragma mark - Authentication

    void SetAuthenticationAgent(ZeldaAuthenticationAgent *agent);

#pragma mark - Launch

    int StartProxy(const ZeldaMode &mode);

    /* Private Methods */

private:

#pragma mark - Logger

    ZeldaLogger *Log = nullptr;

#pragma mark - Options

    bool _use_splice = false;
    int _max_connection = 20;

#pragma mark - Proxies

    int StartTCPProxy(int server_sock, ZeldaMode mode);

#pragma mark - Socket

    std::string _address = "";
    int _port = 0;
    std::string GetSocketString();

    std::string _remote_address = "";
    int _remote_port = 0;
    std::string GetRemoteSocketString();

    int GetServerSock();
    int CreateSocket(const char *bind_addr, int port, int max_connection);

#pragma mark - TCP Client

    void HandleTCPClient(int client_sock, std::string remote_address, int remote_port, bool keep_alive);
    void HandlePlainClient(int client_sock, std::string remote_address, int remote_port, bool keep_alive);
    void HandleTunnelClient(int client_sock);
    int CreateTCPConnection(const char *remote_host, int remote_port, bool keep_alive);

#pragma mark - Tunnel Request

    void HandleTunnelRequest(int source_sock, ZeldaProtocol *protocol);

#pragma mark - Data Forwarding

    void ForwardTCPData(int source_sock, int destination_sock);
    void ForwardProtocolData(int source_sock, int destination_sock, ZeldaProtocol *protocol);

#pragma mark - Helper

    std::string SocketStringFromSocket(sockaddr_in addr);

    int _connections_processed = 0;
    void AddProcessedConnection();
    void ResetProcessedConnection();

#pragma mark - Authentication

    ZeldaAuthenticationAgent *authenticationAgent = nullptr;

};
