#import <iostream>
#import <unistd.h>

#import "cxxopts.hpp"

#import "ZeldaDefines.h"

#import "Zelda.h"
#import "ZeldaLogger.h"

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
                ("log", "Debug level (error/warning/info*/debug)", cxxopts::value<std::string>(default_level), "debug-level")
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

#if defined(ZELDA_USE_SPLICE)
        if (result.count("use-splice"))
        {
            z.SetUseSplice(1);
        }
#endif

        auto zl = new ZeldaLogger(level);
        z->SetLogger(zl);

        zl->Info(title);
        usleep(100000);

        int ret_code = z->StartProxy(mode);

        delete z;
        delete zl;

        return ret_code;

    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
