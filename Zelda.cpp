//
// Created by Zheng on 05/03/2018.
//

#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <sys/types.h>
#import <sys/wait.h>
#import <unistd.h>
#import <netdb.h>
#import <fcntl.h>
#import <errno.h>
#import <climits>

#import "Zelda.h"
#import "ZeldaDefines.h"
#import "ZeldaHTTPTunnel.h"


#define PREAD  0
#define PWRITE 1



#pragma mark - Helpers

/*
 * Convert sockaddr_in to socket string
 */
std::string Zelda::SocketStringFromSocket(sockaddr_in addr)
{
    const char *addr_str = inet_ntoa(addr.sin_addr);
    return std::string(addr_str) + ":" + std::to_string(ntohs(addr.sin_port));
}



#pragma mark - Signal Handler

static int _server_sock = 0;

/* Handle finished child process */
void sigchld_handler(int signal)
{
    while (waitpid(-1, nullptr, WNOHANG) > 0);
}

/* Handle term signal */
void sigterm_handler(int signal)
{
    close(_server_sock);
    exit(0);
}

typedef enum {
    z_address_invalid = 0,
    z_address_ipv4 = 1,
    z_address_ipv6 = 2
} z_address_type;

z_address_type z_check_address(const char *addr)
{
    in_addr s{};
    in6_addr s6{};
    int flag = 0;
    flag = inet_pton(AF_INET, addr, &s); // ipv4?
    if (flag > 0) return z_address_ipv4;
    flag = inet_pton(AF_INET6, addr, &s6); // ipv6
    if (flag > 0) return z_address_ipv6;
    return z_address_invalid;
}



#pragma mark - Initializers

Zelda::Zelda(const std::string &address, int port)
{
    const char *addr = address.c_str();
    z_address_type type = z_check_address(addr);
    if (type == z_address_ipv4) {
        _address = std::string(address);
    } else {
        _address = std::string("127.0.0.1");
    }
    _port = port;
}



#pragma mark - Getters

bool Zelda::GetUseSplice()
{
    return _use_splice;
}

ZeldaLogger *Zelda::GetLogger()
{
    return Log;
}

std::string Zelda::GetAddress()
{
    return _address;
}

int Zelda::GetPort()
{
    return _port;
}

std::string Zelda::GetSocketString()
{
    return GetAddress() + ":" + std::to_string(GetPort());
}

std::string Zelda::GetRemoteAddress()
{
    return _remote_address;
}

int Zelda::GetRemotePort()
{
    return _remote_port;
}

std::string Zelda::GetRemoteSocketString()
{
    return GetRemoteAddress() + ":" + std::to_string(GetRemotePort());
}

int Zelda::GetMaxConnection()
{
    return _max_connection;
}

int Zelda::GetServerSock()
{
    return _server_sock;
}



#pragma mark - Setters

void Zelda::SetUseSplice(bool splice)
{
    _use_splice = splice;
}

void Zelda::SetRemoteAddress(const std::string &address)
{
    const char *addr = address.c_str();
    if (z_check_address(addr) == z_address_ipv4) {
        _remote_address = std::string(address);
    } else {

    }
}

void Zelda::SetRemotePort(int port)
{
    _remote_port = port;
}

void Zelda::SetMaxConnection(int count)
{
    _max_connection = count;
}

void Zelda::SetLogger(ZeldaLogger *logger)
{
    Log = logger;
}

void Zelda::AddProcessedConnection()
{
    _connections_processed++;
}

void Zelda::ResetProcessedConnection()
{
    _connections_processed = 0;
}

void Zelda::SetAuthenticationAgent(ZeldaAuthenticationAgent *agent) {
    authenticationAgent = agent;
}

void Zelda::SetFilterAgent(ZeldaFilterAgent *agent) {
    filterAgent = agent;
}



#pragma mark - Proxies

int Zelda::StartProxy(const ZeldaMode &mode)
{

    signal(SIGCHLD, sigchld_handler); // prevent ended children from becoming zombies
    signal(SIGTERM, sigterm_handler); // handle KILL signal

    if ((_server_sock = CreateSocket(GetAddress().c_str(), GetPort(), GetMaxConnection())) < 0 )
    {
        Log->Fatal("Cannot create new socket");
        return -1;
    }

    Log->Info(mode + " proxy listen on " + GetSocketString() + "...");

    if (mode == ZELDA_MODE_PLAIN || mode == ZELDA_MODE_TUNNEL || mode == ZELDA_MODE_TCP)
    {

        if (mode == ZELDA_MODE_TCP)
        {

            if (GetRemoteAddress().empty()) {
                Log->Fatal("Cannot start " + mode + " proxy without explict destination address");
                return -1;
            }

        }
        else if (mode == ZELDA_MODE_TUNNEL)
        {

            if (!GetRemoteAddress().empty()) {
                Log->Fatal("Cannot start " + mode + " proxy with explict destination address");
                return -1;
            }

        }
        return StartTCPProxy(GetServerSock(), mode);

    }
    else
    {

        Log->Fatal(mode + " proxy not found");

    }

    return -1;

}

int Zelda::StartTCPProxy(int server_sock, ZeldaMode mode)
{

    struct sockaddr_in client_addr {};
    socklen_t addr_len = sizeof(client_addr);

    while (true)
    {

        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);

        if (fork() == 0)
        { // handle client connection in a separated process

            close(server_sock);

            Log->Debug("Receive connection from " + SocketStringFromSocket(client_addr));

            if (mode == ZELDA_MODE_PLAIN)
            {
                HandlePlainClient(client_sock, GetRemoteAddress(), GetRemotePort(), true);
            }
            else if (mode == ZELDA_MODE_TUNNEL)
            {
                HandleTunnelClient(client_sock);
            }
            else
            {
                HandleTCPClient(client_sock, GetRemoteAddress(), GetRemotePort(), true);
            }

            goto proxy_end;

        }
        else
        {
            AddProcessedConnection();
        }

        close(client_sock);

    }

    proxy_end:

    exit(0); return 0;

}



#pragma mark - Socket

int Zelda::CreateSocket(const char *bind_addr, int port, int max_connection)
{

    int server_sock, optval = 1;
    struct sockaddr_in server_addr {};

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        close(server_sock);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (bind_addr == nullptr)
    {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        server_addr.sin_addr.s_addr = inet_addr(bind_addr);
    }

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        close(server_sock);
        Log->Fatal("Cannot bind address " + GetSocketString());
        return -1;
    }

    if (listen(server_sock, max_connection) < 0)
    {
        close(server_sock);
        Log->Fatal("Cannot listen on " + GetSocketString());
        return -1;
    }

    return server_sock;

}



#pragma mark - TCP Client

int Zelda::CreateTCPConnection(const char *remote_host, int remote_port, bool keep_alive)
{

    struct sockaddr_in server_addr {};
    struct hostent *server;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Log->Warning("Cannot create new socket");
        return -1;
    }

    if (keep_alive)
    {
        int optval = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
        {
            close(sock);
            return -1;
        }
    }

    if ((server = gethostbyname(remote_host)) == nullptr)
    {
        close(sock);
        Log->Warning("Cannot resolve address " + std::string(remote_host));
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, static_cast<size_t>(server->h_length));
    server_addr.sin_port = htons(remote_port);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sock);
        Log->Warning("Cannot connect to " + std::string(remote_host) + ":" + std::to_string(remote_port));
        return -1;
    }

    Log->Debug("Connected to " + SocketStringFromSocket(server_addr));
    return sock;

}

void Zelda::HandlePlainClient(int client_sock, std::string remote_address, int remote_port, bool keep_alive)
{

    int remote_sock = -1;
    if (remote_address.empty())
    { // if remote address is empty, do not create tcp connection in this scope.

    }
    else
    {
        remote_sock = CreateTCPConnection(remote_address.c_str(), remote_port, keep_alive);
        if (remote_sock < 0) goto cleanup;
    }

    if (fork() == 0)
    { // a process forwarding data from client to remote socket
        ZeldaProtocol *httpProtocol = new ZeldaHTTPRequest();
        httpProtocol->SetLogger(Log);
        ForwardProtocolData(client_sock, remote_sock, httpProtocol);
        delete(httpProtocol);
        exit(0);
    }

    if (remote_sock >= 0 && fork() == 0)
    { // a process forwarding data from remote socket to client
        ZeldaProtocol *httpProtocol = new ZeldaHTTPResponse();
        httpProtocol->SetLogger(Log);
        ForwardProtocolData(remote_sock, client_sock, httpProtocol);
        delete(httpProtocol);
        exit(0);
    }

    cleanup:

    if (remote_sock >= 0) close(remote_sock);
    if (client_sock >= 0) close(client_sock);

}

void Zelda::HandleTunnelClient(int client_sock)
{

    if (fork() == 0) { // a process which handles tunnel request
        auto *tunnelProtocol = new ZeldaHTTPTunnel();
        tunnelProtocol->SetAuthenticationAgent(authenticationAgent);
        tunnelProtocol->SetFilterAgent(filterAgent);

        ZeldaProtocol *httpProtocol = tunnelProtocol;
        httpProtocol->SetLogger(Log);
        HandleTunnelRequest(client_sock, httpProtocol);
        delete(tunnelProtocol);
        exit(0);
    }

    if (client_sock >= 0) close(client_sock);

}

void Zelda::HandleTCPClient(int client_sock, std::string remote_address, int remote_port, bool keep_alive)
{

    int remote_sock = -1;
    if (!remote_address.empty())
    { // tcp proxy needs an explicit remote address
        remote_sock = CreateTCPConnection(remote_address.c_str(), remote_port, keep_alive);
        if (remote_sock < 0) goto cleanup;
    }

    if (fork() == 0)
    { // a process forwarding data from client to remote socket
        ForwardTCPData(client_sock, remote_sock);
        exit(0);
    }

    if (fork() == 0)
    { // a process forwarding data from remote socket to client
        ForwardTCPData(remote_sock, client_sock);
        exit(0);
    }

    cleanup:

    if (remote_sock >= 0) close(remote_sock);
    if (client_sock >= 0) close(client_sock);

}

#pragma mark - Tunnel Request

void Zelda::HandleTunnelRequest(int source_sock, ZeldaProtocol *protocol)
{

    int destination_sock = -1;
    if (!protocol || source_sock < 0) goto forward_clean;

    ssize_t n;
    char *buffer[ZELDA_BUF_SIZE];

    while ((n = recv(source_sock, buffer, ZELDA_BUF_SIZE, 0)) > 0)
    { // read data from input socket

        auto buflen = static_cast<uint64_t>(n);
        auto *buf = (char *)malloc(buflen);

        if (buf == nullptr)
        {
            goto forward_clean;
        }

        memcpy(buf, buffer, buflen);

        size_t newlen = buflen;
        protocol->processChuck(&buf, &newlen);

        if (buf == nullptr)
        {
            goto forward_clean;
        }

        if (protocol->GetRemoteAddress().empty() || protocol->GetRemotePort() == 0)
        {
            // do not process following packets
            free(buf);
            break;
        }

        if (protocol->isHandled())
        { // if request is handled by tunnel protocol

            if (protocol->isActive() && fork() == 0)
            { // build tcp bridge in a separated process
                HandleTCPClient(source_sock, protocol->GetRemoteAddress(), protocol->GetRemotePort(), protocol->shouldKeepAlive());
                exit(0);
            }

            // handle tunnel request
            send(source_sock, buf, newlen, 0);
            Log->Debug(protocol->description() + std::to_string(newlen) + " bytes");

            // do not process following packets
            free(buf);
            break;

        }

        if (destination_sock < 0)
        { // downgrade to plain proxy

            Log->Debug(protocol->description() + "Downgrade to plain mode");

            int remote_sock =
                    CreateTCPConnection(protocol->GetRemoteAddress().c_str(), protocol->GetRemotePort(), protocol->shouldKeepAlive());
            if (remote_sock < 0) {
                free(buf);
                break;
            }

            if (fork() == 0)
            { // a process forwarding data from remote socket to client
                ZeldaProtocol *httpProtocol = new ZeldaHTTPResponse();
                httpProtocol->SetLogger(Log);
                ForwardProtocolData(remote_sock, source_sock, httpProtocol);
                delete(httpProtocol);
                exit(0);
            }

            destination_sock = remote_sock;

        }

        // forward directly
        send(destination_sock, buf, newlen, 0);
        Log->Debug(protocol->description() + std::to_string(newlen) + " bytes");

        free(buf);

        if (!protocol->isActive()) {
            break;
        }

    }

    forward_clean:

    if (source_sock >= 0) close(source_sock);

}

#pragma mark - Data Forwarding

void Zelda::ForwardTCPData(int source_sock, int destination_sock)
{

    bool use_splice = _use_splice;

    ssize_t n;
    if (use_splice) {

#if defined(ZELDA_USE_SPLICE)

        int buf_pipe[2];

        if (pipe(buf_pipe) == -1) {
            Log->Warning("Cannot create pipe");
            return;
        }

        while ((n = splice(source_sock, NULL, buf_pipe[PWRITE], NULL, SSIZE_MAX, SPLICE_F_NONBLOCK | SPLICE_F_MOVE)) > 0) {
            if (splice(buf_pipe[PREAD], NULL, destination_sock, NULL, SSIZE_MAX, SPLICE_F_MOVE) < 0) {
                Log->Warning("Cannot write to socket");
                return;
            }
        }

        if (n < 0) {
            Log->Warning("Cannot read from socket");
        }

        close(buf_pipe[0]);
        close(buf_pipe[1]);

#endif

    } else {

        char *buffer[ZELDA_BUF_SIZE];

        while ((n = recv(source_sock, buffer, ZELDA_BUF_SIZE, 0)) > 0) { // read data from input socket
            send(destination_sock, buffer, static_cast<size_t>(n), 0); // send data to output socket
        }

        if (n < 0) {
            Log->Warning("Cannot read from socket");
        }

    }

    shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
    close(destination_sock);

    shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
    close(source_sock);

}

void Zelda::ForwardProtocolData(int source_sock, int destination_sock, ZeldaProtocol *protocol)
{

    if (!protocol || source_sock < 0) goto forward_clean;

    ssize_t n;
    char *buffer[ZELDA_BUF_SIZE];

    while ((n = recv(source_sock, buffer, ZELDA_BUF_SIZE, 0)) > 0)
    { // read data from input socket

        auto buflen = static_cast<uint64_t>(n);
        auto *buf = (char *)malloc(buflen);

        if (buf == nullptr)
        {
            goto forward_clean;
        }

        memcpy(buf, buffer, buflen);

        size_t newlen = buflen;
        protocol->processChuck(&buf, &newlen);

        if (buf == nullptr)
        {
            goto forward_clean;
        }

        if (destination_sock < 0)
        {

            if (protocol->GetRemoteAddress().empty() || protocol->GetRemotePort() == 0)
            {
                free(buf);
                break;
            }

            int remote_sock =
                    CreateTCPConnection(protocol->GetRemoteAddress().c_str(), protocol->GetRemotePort(), protocol->shouldKeepAlive());
            if (remote_sock < 0) {
                free(buf);
                goto forward_clean;
            }

            if (fork() == 0)
            { // a process forwarding data from remote socket to client

                ZeldaProtocol *httpProtocol = new ZeldaHTTPResponse();
                httpProtocol->SetLogger(Log);
                ForwardProtocolData(remote_sock, source_sock, httpProtocol);
                delete(httpProtocol);
                exit(0);

            }

            destination_sock = remote_sock;

        }

        // forward directly
        send(destination_sock, buf, newlen, 0);
        Log->Debug(protocol->description() + std::to_string(newlen) + " bytes");

        free(buf);

        if (!protocol->isActive()) {
            break;
        }

    }

    if (n < 0)
    {
        Log->Warning("Cannot read from socket");
        goto forward_clean;
    }

    forward_clean:

    if (destination_sock >= 0)
    {
        shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
        close(destination_sock);
    }

    if (source_sock >= 0)
    {
        shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
        close(source_sock);
    }

}

