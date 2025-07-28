//
// Created by ywj on 2025/7/29.
//

#ifndef UNIXNET_CONFIG_H
#define UNIXNET_CONFIG_H

#include<memory>
#include<string>
#include<vector>
#include<filesystem>

struct LogicHost
{
    std::string name;
    std::string host;
    size_t conns;
};

class config
{
public:
    config(const config&) = delete;
    config& operator=(const config&) = delete;
    config(config&&) = delete;
    config& operator=(config&&) = delete;
    static config& instance();
    static bool loadConfig(const std::filesystem::path& path);
    [[nodiscard]] std::vector<LogicHost> getLogicHosts() const;
    [[nodiscard]] std::string getBindAddr() const;
    [[nodiscard]] short getBindPort() const;
    void setBindPort(short port);
    void setBindAddr(const std::string& addr);
    void setLogicHosts(const std::vector<LogicHost>& hosts);

protected:
    config()=default;
    ~config()=default;
private:
    std::string _bind_addr;
    short _bind_port{};
    std::vector<LogicHost> _logic_hosts;
    static std::shared_ptr<config> _instance;
};


#endif //UNIXNET_CONFIG_H