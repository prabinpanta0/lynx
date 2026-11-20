#pragma once

/****************************************************************************
 *  ____   ___   ____ _  _______ _____    ____ ___  _   _ _____ ___ ____    *
 * / ___| / _ \ / ___| |/ / ____|_   _|  / ___/ _ \| \ | |  ___|_ _/ ___|   *
 * \___ \| | | | |   | ' /|  _|   | |   | |  | | | |  \| | |_   | | |  _    *
 *  ___) | |_| | |___| . \| |___  | |   | |__| |_| | |\  |  _|  | | |_| |   *
 * |____/ \___/ \____|_|\_\_____| |_|    \____\___/|_| \_|_|   |___\____|   *
 *                                                                          *
 * Socket configuration for domain, type, protocol, and address.            *
 ****************************************************************************/

#include "net/socket_address.hpp"
#include "net/socket_type.hpp"
namespace lynx::net
{
	/*
	 * Defines the native socket settings and local address.  
	 * Defaults to an IPv4 TCP stream socket on 0.0.0.0:8080. 
	 */
	struct socket_config
	{
		socket_domain domain{socket_domain::IPv4};
		socket_type type{socket_type::STREAM};
		socket_protocol protocol{socket_protocol::DEFAULT};
		socket_address sock_addr{.ip = "0.0.0.0", .port = 8080};
	};
} // namespace lynx::net