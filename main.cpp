#import <stdio.h>
#import <stdlib.h>
#import <unistd.h>
#import <signal.h>
#import <fcntl.h>
#import <ctype.h>

#import <sys/stat.h>
#import <sys/param.h>
#import <sys/types.h>
#import <sys/wait.h>
#import <sys/syscall.h>

#import <iostream>

#import "cxxopts.hpp"

#import "ZeldaDefines.h"
#import "Zelda.h"
#import "ZeldaLogger.h"
#import "ZeldaHTTPHelper.h"

int main(int argc, char* argv[])
{
    try
    {

        std::string title(ZELDA_NAME);
        title += " ";
        title += ZELDA_VERSION;

        std::string intro(title);
        intro += " - ";
        intro += "Lightweight HTTP Proxy";

        auto *zl = new ZeldaLogger();
        zl->Info(title);

        cxxopts::Options options(argv[0], intro);

        std::string default_addr(ZELDA_ADDRESS);
        std::string default_port(ZELDA_PORT);
        std::string default_level(ZELDA_LOG_INFO);
        std::string default_mode("tunnel");
        std::string default_max_connection("100");

        options.add_options(ZELDA_NAME)
                ("a,address", "Bind address (127.0.0.1)", cxxopts::value<std::string>(default_addr), "address")
                ("p,port", "Bind port (10087)", cxxopts::value<std::string>(default_port), "port")
                ("remote-address", "Remote address", cxxopts::value<std::string>(), "address")
                ("remote-port", "Remote port", cxxopts::value<std::string>(), "port")
                ("max-connection", "Max connection (20)", cxxopts::value<std::string>(default_max_connection), "max-num")
#if defined(ZELDA_USE_SPLICE)
                ("use-splice", "Use splice to boost data copy")
#endif
                ("mode", "Proxy mode (tunnel*/plain/tcp)", cxxopts::value<std::string>(default_mode), "proxy-mode")
                ("auth", "Authentication list", cxxopts::value<std::string>(), "list-path")
                ("log", "Debug level (error/warning/info*/debug)", cxxopts::value<std::string>(default_level), "debug-level")
                ("daemonize", "Run in background")
                ("version", "Print version")
                ("help", "Print help");

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", ZELDA_NAME}) << std::endl;
            return 0;
        }
        else if (result.count("version"))
        {
            std::cout << title << std::endl;
            return 0;
        }

        std::string address(default_addr);
        std::string port(default_port);
        std::string level(default_level);
        std::string mode(default_mode);
        std::string max_connection(default_max_connection);

        if (result.count("a"))
            address = result["a"].as<std::string>();

        if (result.count("p"))
            port = result["p"].as<std::string>();

        if (result.count("log"))
            level = result["log"].as<std::string>();

        if (result.count("mode"))
            mode = result["mode"].as<std::string>();

        if (result.count("max-connection"))
            max_connection = result["max-connection"].as<std::string>();

        int port_n = std::stoi(port);
        int max_connection_n = std::stoi(max_connection);

        auto *z = new Zelda(address, port_n);
        z->SetMaxConnection(max_connection_n);

        if (result.count("remote-address"))
        {
            std::string remote_address = result["remote-address"].as<std::string>();
            z->SetRemoteAddress(remote_address);
        }

        if (result.count("remote-port"))
        {
            z->SetRemotePort(std::stoi(result["remote-port"].as<std::string>()));
        }

        ZeldaAuthenticationAgent *agent = nullptr;
        if (result.count("auth"))
        {
            std::string list_path = result["auth"].as<std::string>();
            std::list<std::string> auth_list = ZeldaHTTPHelper::authenticationListAtPath(list_path.c_str());
            auto *auth_agent = new ZeldaAuthenticationAgent();
            auth_agent->authenticationList = auth_list;
            z->SetAuthenticationAgent(auth_agent);
            zl->Info("Found " + std::to_string(auth_list.size()) + " entries in authentication database");
            if (auth_list.empty()) {
                zl->Error("Authentication database is empty, and no income connection will be allowed");
            }
            agent = auth_agent;
        }

#if defined(ZELDA_USE_SPLICE)
        if (result.count("use-splice"))
        {
            z->SetUseSplice(1);
        }
#endif

        zl->SetLogLevel(level);
        z->SetLogger(zl);

        usleep(100000);

        bool daemonize = false;
        if (result.count("daemonize")) {
            daemonize = true;
        }

        if (daemonize) {
            pid_t pid;
            int i;

            // [1] fork child process and exit father process
            if ((pid = fork()) != 0) {
                exit(EXIT_SUCCESS);
            } else if (pid < 0) {
                exit(EXIT_FAILURE);
            }

            // [2] create a new session
            setsid();

            if ((pid = fork()) != 0) {
                exit(EXIT_SUCCESS);
            } else if (pid < 0) {
                exit(EXIT_FAILURE);
            }

            // [3] set current path
            char szPath[PATH_MAX];
            if(getcwd(szPath, sizeof(szPath)) == nullptr)
            {
                perror("getcwd");
                exit(EXIT_FAILURE);
            }
            else
            {
                chdir(szPath);
                pid = getpid();
                zl->Warning("(" + std::to_string(pid) + ") Zelda Daemon is running...");

                FILE *fp = fopen("daemon.pid", "w");
                if (!fp) {
                    perror("fopen");
                    exit(EXIT_FAILURE);
                }
                fprintf(fp, "%d", pid);
                fclose(fp);
            }

            // [4] umask 0
            umask(0);

            // [5] close useless fd
            for (i = 0; i < NOFILE; ++i)
                close(i);

            // [6] set termianl signal (skip)

        }

        // [7] start main proxy
        int ret_code = z->StartProxy(mode);

        delete(agent);
        delete(z);
        delete(zl);

        return ret_code;

    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
