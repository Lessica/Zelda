//
// Created by Zheng on 05/03/2018.
//

#import <unistd.h>
#import <netdb.h>

#import "Zelda.h"
#import "ZeldaDefines.h"


#define PREAD  0
#define PWRITE 1



#pragma mark - Helpers

/*
 * Convert sockaddr_in to socket string
 */
std::string Zelda::SocketStringFromSocket(sockaddr_in addr) {
    const char *addr_str = inet_ntoa(addr.sin_addr);
    return std::string(addr_str) + ":" + std::to_string(ntohs(addr.sin_port));
}



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

typedef enum {
    z_address_invalid = 0,
    z_address_ipv4 = 1,
    z_address_ipv6 = 2
} z_address_type;

z_address_type z_check_address(const char *addr) {
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

Zelda::Zelda(const std::string &address, int port) {
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

bool Zelda::GetUseSplice() {
    return _use_splice;
}

ZeldaLogger *Zelda::GetLogger() {
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

void Zelda::SetUseSplice(bool splice) {
    _use_splice = splice;
}

void Zelda::SetRemoteAddress(const std::string &address) {
    const char *addr = address.c_str();
    if (z_check_address(addr) == z_address_ipv4) {
        _remote_address = std::string(address);
    } else {

    }
}

void Zelda::SetRemotePort(int port) {
    _remote_port = port;
}

void Zelda::SetMaxConnection(int count) {
    _max_connection = count;
}

void Zelda::SetLogger(ZeldaLogger *logger) {
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
        Log->Fatal("Cannot create new socket");
        return -1;
    }

    Log->Info("\"" + mode + "\" proxy listen on " + GetSocketString() + "...");

    if (mode == ZELDA_MODE_PLAIN || mode == ZELDA_MODE_TUNNEL || mode == ZELDA_MODE_TCP) {
        if (mode == ZELDA_MODE_TCP) {
            if (GetAddress().empty()) {
                Log->Fatal("Cannot start \"" + mode + "\" proxy without explict destination address");
                return -1;
            }
        }
        return StartTCPProxy(GetServerSock(), mode);
    } else {
        Log->Fatal("\"" + mode + "\" proxy not found");
    }

    return -1;
}

int Zelda::StartTCPProxy(int server_sock, ZeldaMode mode) {
    struct sockaddr_in client_addr {};
    socklen_t addr_len = sizeof(client_addr);

    while (true) {

        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);

        if (fork() == 0) { // handle client connection in a separate process

            close(server_sock);

            Log->Debug("Receive connection from " + SocketStringFromSocket(client_addr));

            if (mode == ZELDA_MODE_PLAIN) {
                HandlePlainClient(client_sock);
            } else if (mode == ZELDA_MODE_TUNNEL) {
                HandleTunnelClient(client_sock);
            } else {
                HandleTCPClient(client_sock);
            }

            goto proxy_end;

        } else {
            AddProcessedConnection();
        }

        close(client_sock);

    }

    proxy_end:
    exit(0); return 0;
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
        Log->Fatal("Cannot bind address " + GetSocketString());
        return -1;
    }

    if (listen(server_sock, max_connection) < 0) {
        close(server_sock);
        Log->Fatal("Cannot listen on " + GetSocketString());
        return -1;
    }

    return server_sock;
}



#pragma mark - TCP Client

int Zelda::CreateTCPConnection(const char *remote_host, int remote_port) {
    return CreateTCPConnection(remote_host, remote_port, false);
}

int Zelda::CreateTCPConnection(const char *remote_host, int remote_port, bool keep_alive) {

    struct sockaddr_in server_addr {};
    struct hostent *server;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        Log->Error("Cannot create new socket");
        return -1;
    }

    if (keep_alive) {
        int optval = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
            close(sock);
            return -1;
        }
    }

    if ((server = gethostbyname(remote_host)) == nullptr) {
        close(sock);
        Log->Error("Cannot resolve address " + GetRemoteAddress());
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, static_cast<size_t>(server->h_length));
    server_addr.sin_port = htons(remote_port);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        Log->Error("Cannot connect to " + GetRemoteSocketString());
        return -1;
    }

    Log->Debug("Connected to " + SocketStringFromSocket(server_addr));
    return sock;
}

void Zelda::HandlePlainClient(int client_sock) {

    int remote_sock = -1;
    std::string remote_address = GetRemoteAddress();
    if (!remote_address.empty()) {
        remote_sock = CreateTCPConnection(remote_address.c_str(), GetRemotePort());
        if (remote_sock < 0) goto cleanup;
    }

    if (fork() == 0) { // a process forwarding data from client to remote socket
        ZeldaProtocol *httpProtocol = new ZeldaHTTPRequest();
        httpProtocol->SetLogger(Log);
        ForwardProtocolData(client_sock, remote_sock, httpProtocol);
        delete(httpProtocol);
        exit(0);
    }

    if (remote_sock >= 0 && fork() == 0) { // a process forwarding data from remote socket to client
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

void Zelda::HandleTunnelClient(int client_sock) {

}

void Zelda::HandleTCPClient(int client_sock) {

    std::string remote_address = GetRemoteAddress();
    int remote_sock = CreateTCPConnection(remote_address.c_str(), GetRemotePort());
    if (remote_sock < 0) goto cleanup;

    if (fork() == 0) { // a process forwarding data from client to remote socket
        ForwardTCPData(client_sock, remote_sock);
        exit(0);
    }

    if (fork() == 0) { // a process forwarding data from remote socket to client
        ForwardTCPData(remote_sock, client_sock);
        exit(0);
    }

    cleanup:
    if (remote_sock >= 0) close(remote_sock);
    if (client_sock >= 0) close(client_sock);

}



#pragma mark - Data Forwarding

void Zelda::ForwardTCPData(int source_sock, int destination_sock) {
    bool use_splice = _use_splice;

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

#endif

    } else {

        char *buffer[ZELDA_BUF_SIZE];

        while ((n = recv(source_sock, buffer, ZELDA_BUF_SIZE, 0)) > 0) { // read data from input socket
            send(destination_sock, buffer, static_cast<size_t>(n), 0); // send data to output socket
        }

        if (n < 0) {
            Log->Error("Cannot read from socket");
        }

    }

    shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
    close(destination_sock);

    shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
    close(source_sock);
}

void Zelda::ForwardProtocolData(int source_sock, int destination_sock, ZeldaProtocol *protocol) {
    if (!protocol || source_sock < 0) goto forward_clean;

    ssize_t n;
    char *buffer[ZELDA_BUF_SIZE];
    while ((n = recv(source_sock, buffer, ZELDA_BUF_SIZE, 0)) > 0) { // read data from input socket
        auto buflen = static_cast<uint64_t>(n);
        auto *buf = (char *)malloc(buflen);
        memcpy(buf, buffer, buflen);

        size_t newlen = buflen;
        protocol->processChuck(&buf, &newlen);

        if (buf == nullptr) {
            goto forward_clean;
        }

        if (destination_sock < 0) {
            int remote_sock = -1;
            std::string remote_address = protocol->GetRemoteAddress();
            if (remote_address.empty()) goto forward_clean;
            remote_sock = CreateTCPConnection(remote_address.c_str(), protocol->GetRemotePort(), protocol->shouldKeepAlive());
            if (remote_sock < 0) goto forward_clean;
            if (fork() == 0) { // a process forwarding data from remote socket to client
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

        delete(buf);
    }

    if (n < 0) {
        Log->Error("Cannot read from socket");
        goto forward_clean;
    }

    forward_clean:
    if (destination_sock >= 0) {
        shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
        close(destination_sock);
    }
    if (source_sock >= 0) {
        shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
        close(source_sock);
    }
}


