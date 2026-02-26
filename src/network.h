#ifndef __NETWORK_H_
#define __NETWORK_H_

#include <string>
#include <mutex>

#include <stdint.h>
#include "json.hpp"


#define PATH(_target_)			(SYSTEM_PATH_##_target_)

#define SYSTEM_PATH_NET_INTERFACES	"/etc/network/interfaces"

#define	DEFAULT_IP			"192.168.1.100"
#define DEFAULT_NETMASK			"255.255.255.0"
#define DEFAULT_GATEWAY			"192.168.1.1"

#define ETH_LOOPBACK 			"# Configure Loopback\nauto lo\niface lo inet loopback\n\nauto eth0\n"
#define ETH_DHCP			"iface eth0 inet dhcp"
#define ETH_STATIC			"iface eth0 inet static"
#define ETH_FALLBACK			"auto eth0:1"
#define ETH_FALLBACK_STATIC		"iface eth0:1 inet static"

using json = nlohmann::json;

class Network
{
private:
	bool m_dhcp;
	std::string m_dns1;
	std::string m_dns2;
	std::string m_ipAddress;
	std::string m_netmask;
	std::string m_gateway;
	
public:
	Network();
	~Network();
	
	void save();
	
	void fromJson(const json &jsonObject);
	json getJson();

	void setDhcp(bool enable);
	void setDns(const std::string &dns);
	void setDns(const std::string &dns1, const std::string &dns2);
	void setGateway(const std::string &gateway);
	void setIpAddress(const std::string &ipAddress);
	void setNetmask(const std::string &netmask);
	
	bool getDhcp();
};

#endif
