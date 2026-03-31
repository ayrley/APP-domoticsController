#include <cerrno>
#include <fstream>
#include <iostream>
#include <string>

#include <stdint.h>

#include "network.h"
#include "reader.h"

Network::Network()
{
}

Network::~Network()
{
}

void Network::fromJson(const json &jsonObject)
{
    this->m_netmask = jsonObject.value("netmask", "");
    this->m_gateway = jsonObject.value("gateway", "");
    this->m_ipAddress = jsonObject.value("ipAddress", "");
    this->m_dns1 = jsonObject.value("dns1", "");
    this->m_dns2 = jsonObject.value("dns2", "");
    this->m_dhcp = jsonObject.value("dhcp", true);
}

json Network::getJson()
{
    json object = {};
    object += json::object_t::value_type("netmask", this->m_netmask);
    object += json::object_t::value_type("gateway", this->m_gateway);
    object += json::object_t::value_type("ipAddress", this->m_ipAddress);
    object += json::object_t::value_type("dns1", this->m_dns1);
    object += json::object_t::value_type("dns2", this->m_dns2);
    object += json::object_t::value_type("dhcp", this->m_dhcp);

    return object;
}

void Network::save()
{
    std::ofstream interfacesFp(PATH(NET_INTERFACES));

    interfacesFp << ETH_LOOPBACK << std::endl;

    if (this->m_dhcp) {
        interfacesFp << ETH_DHCP << std::endl;
    } else {
        interfacesFp << ETH_STATIC << std::endl;
        interfacesFp << " address " << this->m_ipAddress << std::endl;
        interfacesFp << " netmask " << this->m_netmask << std::endl;
        if (!this->m_gateway.empty())
            interfacesFp << " gateway " << this->m_gateway << std::endl;
    }

    if (!this->m_dns1.empty() || !this->m_dns2.empty())
        interfacesFp << "dns-nameserver ";

    if (!this->m_dns1.empty())
        interfacesFp << this->m_dns1 << " ";

    if (!this->m_dns2.empty())
        interfacesFp << this->m_dns2;

    interfacesFp << std::endl;

    if (this->m_dhcp) {
        interfacesFp << std::endl;
        interfacesFp << ETH_FALLBACK << std::endl;
        interfacesFp << ETH_FALLBACK_STATIC << std::endl;
        interfacesFp << "address " << DEFAULT_IP << std::endl;
        interfacesFp << "netmask " << DEFAULT_NETMASK << std::endl;
    }

    interfacesFp.close();
}

bool Network::getDhcp()
{
    return this->m_dhcp;
}

void Network::setDhcp(bool enable)
{
    this->m_dhcp = enable;
}

void Network::setDns(const std::string &dns)
{
    this->m_dns1 = dns;
}

void Network::setDns(const std::string &dns1, const std::string &dns2)
{
    this->m_dns1 = dns1;
    this->m_dns2 = dns2;
}

void Network::setGateway(const std::string &gateway)
{
    this->m_gateway = gateway;
}

void Network::setIpAddress(const std::string &ipAddress)
{
    this->m_ipAddress = ipAddress;
}

void Network::setNetmask(const std::string &netmask)
{
    this->m_netmask = netmask;
}
