//
// Created by Zheng on 05/03/2018.
//

#include <arpa/inet.h>

#include <unistd.h>
#include <netdb.h>
#include <climits>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include "Zelda.h"
#include "ZeldaDefines.h"

#define ZELDA_BUF_SIZE 16384
#define PREAD  0
#define PWRITE 1

#pragma mark - Signal Handler

static int _server_sock = 0;

/* Handle finished child process */
void sigchld_handler(int signal) {
    while (waitpid(-1, nullptr, WNOHANG) > 0);
}

/* Handle term signal */
void sigterm_handler(int signal) {
    close(_server_sock);
    exit(0);
}

#pragma mark - Initializers

Zelda::Zelda(const std::string &address, int port) {
    const char *addr = address.c_str();
    in_addr_t inaddr = inet_addr(addr);
    if (inaddr != INADDR_NONE) {
        _address = std::string(address);
    } else {
        _address = std::string("127.0.0.1");
    }
    _port = port;
}

#pragma mark - Getters

int Zelda::GetUseSplice() {
    return _use_splice;
}

ZeldaLogger Zelda::GetLogger() {
    return Log;
}

std::string Zelda::GetAddress() {
    return _address;
}

int Zelda::GetPort() {
    return _port;
}

std::string Zelda::GetSocketString() {
    return GetAddress() + ":" + std::to_string(GetPort());
}

std::string Zelda::GetRemoteAddress() {
    return _remote_address;
}

int Zelda::GetRemotePort() {
    return _remote_port;
}

std::string Zelda::GetRemoteSocketString() {
    return GetRemoteAddress() + ":" + std::to_string(GetRemotePort());
}

int Zelda::GetMaxConnection() {
    return _max_connection;
}

int Zelda::GetServerSock() {
    return _server_sock;
}

#pragma mark - Setters

void Zelda::SetUseSplice(int splice) {
    _use_splice = splice;
}

void Zelda::SetRemoteAddress(const std::string &address) {
    _remote_address = std::string(address);
}

void Zelda::SetRemotePort(int port) {
    _remote_port = port;
}

void Zelda::SetMaxConnection(int count) {
    _max_connection = count;
}

void Zelda::SetLogger(ZeldaLogger logger) {
    Log = logger;
}

void Zelda::AddProcessedConnection() {
    _connections_processed++;
}

void Zelda::ResetProcessedConnection() {
    _connections_processed = 0;
}

#pragma mark - Proxies

int Zelda::StartProxy(const ZeldaMode &mode) {

    signal(SIGCHLD, sigchld_handler); // prevent ended children from becoming zombies
    signal(SIGTERM, sigterm_handler); // handle KILL signal

    if ((_server_sock = CreateSocket(GetAddress().c_str(), GetPort(), GetMaxConnection())) < 0 ) {
        Log.Fatal(Log.S() + "Cannot create new socket");
        return -1;
    }

    Log.Info(Log.S()
             + "\"" + mode + "\" proxy listen on " + GetSocketString() + "...");

    if (mode == ZELDA_MODE_PLAIN) {
        return StartPlainProxy();
    } else if (mode == ZELDA_MODE_TUNNEL) {
        return StartTunnelProxy();
    } else if (mode == ZELDA_MODE_TCP) {
        return StartTCPProxy(GetServerSock());
    }

    return -1;

}

int Zelda::StartPlainProxy() {

    return 0;
}

int Zelda::StartTunnelProxy() {

    return 0;
}

int Zelda::StartTCPProxy(int server_sock) {
    struct sockaddr_in client_addr {};
    socklen_t addr_len = sizeof(client_addr);

    while (true) {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (fork() == 0) { // handle client connection in a separate process
            close(server_sock);
            HandleTCPClient(client_sock, client_addr);
            exit(0);
        } else {
            AddProcessedConnection();
        }
        close(client_sock);
    }
}

#pragma mark - Socket

int Zelda::CreateSocket(const char *bind_addr, int port, int max_connection) {
    int server_sock, optval = 1;
    struct sockaddr_in server_addr {};

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        close(server_sock);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (bind_addr == nullptr) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        server_addr.sin_addr.s_addr = inet_addr(bind_addr);
    }

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        close(server_sock);
        Log.Fatal(Log.S() + "Cannot bind address " + GetSocketString());
        return -1;
    }

    if (listen(server_sock, max_connection) < 0) {
        close(server_sock);
        Log.Fatal(Log.S() + "Cannot listen on " + GetSocketString());
        return -1;
    }

    return server_sock;
}

#pragma mark - TCP Client

void Zelda::HandleTCPClient(int client_sock, struct sockaddr_in client_addr) {

    Log.Debug(Log.S() + "Receive connection from " + SocketStringFromSocket(client_addr));

    int remote_sock;
    if ((remote_sock = CreateTCPConnection(GetRemoteAddress().c_str(), GetRemotePort())) < 0) {
        goto cleanup;
    }

    if (fork() == 0) { // a process forwarding data from client to remote socket
        ForwardTCPData(client_sock, remote_sock);
        exit(0);
    }

    if (fork() == 0) { // a process forwarding data from remote socket to client
        ForwardTCPData(remote_sock, client_sock);
        exit(0);
    }

    cleanup:
    close(remote_sock);
    close(client_sock);

}

int Zelda::CreateTCPConnection(const char *remote_host, int remote_port) {
    struct sockaddr_in server_addr {};
    struct hostent *server;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        Log.Error(Log.S() + "Cannot create new socket");
        return -1;
    }

    if ((server = gethostbyname(remote_host)) == nullptr) {
        Log.Error(Log.S() + "Cannot resolve address " + GetRemoteAddress());
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, static_cast<size_t>(server->h_length));
    server_addr.sin_port = htons(remote_port);

    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        Log.Error(Log.S() + "Cannot connect to " + GetRemoteSocketString());
        return -1;
    }

    Log.Debug(Log.S() + "Connected to " + SocketStringFromSocket(server_addr));
    return sock;
}

void Zelda::ForwardTCPData(int source_sock, int destination_sock) {
    int use_splice = _use_splice;
    ssize_t n;

    if (use_splice) {
#if defined(ZELDA_USE_SPLICE)

        int buf_pipe[2];

        if (pipe(buf_pipe) == -1) {
            Log.Error(Log.S() + "Cannot create pipe");
            return;
        }

        while ((n = splice(source_sock, NULL, buf_pipe[PWRITE], NULL, SSIZE_MAX, SPLICE_F_NONBLOCK | SPLICE_F_MOVE)) > 0) {
            if (splice(buf_pipe[PREAD], NULL, destination_sock, NULL, SSIZE_MAX, SPLICE_F_MOVE) < 0) {
                Log.Error(Log.S() + "Cannot write to socket");
                return;
            }
        }

        if (n < 0) {
            Log.Error(Log.S() + "Cannot read from socket");
        }

        close(buf_pipe[0]);
        close(buf_pipe[1]);

        shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
        close(destination_sock);

        shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
        close(source_sock);

        return;

#endif
    } else {

        char buffer[ZELDA_BUF_SIZE];

        while ((n = recv(source_sock, buffer, ZELDA_BUF_SIZE, 0)) > 0) { // read data from input socket
            send(destination_sock, buffer, static_cast<size_t>(n), 0); // send data to output socket
        }

        if (n < 0) {
            Log.Error(Log.S() + "Cannot read from socket");
        }

        shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
        close(destination_sock);

        shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
        close(source_sock);

        return;

    }
}

#pragma mark - Helpers

std::string Zelda::SocketStringFromSocket(sockaddr_in addr) {
    const char *addr_str = inet_ntoa(addr.sin_addr);
    return std::string(addr_str) + ":" + std::to_string(ntohs(addr.sin_port));
}
