//
// Created by root on 2025/7/29.
//

#include "config.h"
#include "yaml-cpp/yaml.h"
#include "log.h"
config& config::instance()
{
    static config instance;
    return instance;
}

bool config::loadConfig(const std::filesystem::path& path)
{
    config&config = instance();
    try
    {
        YAML::Node yaml= YAML::LoadFile(path);
        std::vector<LogicHost>logicHosts;
        auto bindAddr=yaml["listen"]["host"].as<std::string>();
        auto bindPort=yaml["listen"]["port"].as<short>();
        for (const auto& logicHost : yaml["logical_hosts"])
        {
            logicHosts.emplace_back(
                logicHost["host"].as<std::string>(),
                logicHost["name"].as<std::string>()
                ,logicHost["connect"].as<size_t>());
        }
        config.setLogicHosts(logicHosts);
        config.setBindAddr(bindAddr);
        config.setBindPort(bindPort);
        LOG(LOG_INFO, "load config success!","")
        return true;
    }
    catch (const std::exception& e)
    {
       LOG(LOG_ERR, "load config occurred exception: %s", e.what());
        return false;
    }
}

std::vector<LogicHost> config::getLogicHosts() const
{
    return _logic_hosts;
}

std::string config::getBindAddr() const
{
return _bind_addr;
}

short config::getBindPort() const
{
return _bind_port;
}

void config::setBindPort(short port)
{
    _bind_port=port;
}

void config::setBindAddr(const std::string& addr)
{
    _bind_addr=addr;
}

void config::setLogicHosts(const std::vector<LogicHost>& hosts)
{
    _logic_hosts=hosts;
}
