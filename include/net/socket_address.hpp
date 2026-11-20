#pragma once

/**********************************************************
 *  ____   ___   ____ _  _______ _____                    *
 * / ___| / _ \ / ___| |/ / ____|_   _|                   *
 * \___ \| | | | |   | ' /|  _|   | |                     *
 *  ___) | |_| | |___| . \| |___  | |                     *
 * |____/ \___/ \____|_|\_\_____| |_|                     *
 *                                                        *
 *     _    ____  ____  ____  _____ ____ ____             *
 *    / \  |  _ \|  _ \|  _ \| ____/ ___/ ___|            *
 *   / _ \ | | | | | | | |_) |  _| \___ \___ \            *
 *  / ___ \| |_| | |_| |  _ <| |___ ___) |__) |           *
 * /_/   \_\____/|____/|_| \_\_____|____/____/            *
 *                                                        *
 * IPv4 socket address representation and validation.     *
 **********************************************************/

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
namespace lynx::net
{
	/********************************************************
	 * Represents an IPv4 address and port for socket use.  *
	 ********************************************************/
	struct socket_address
	{
		std::string ip{"0.0.0.0"};
		uint16_t port{8080};

		/****************************************************
		 * Converts this address into its native sockaddr.  *
		 ****************************************************/
		[[nodiscard]] sockaddr_in to_sockaddr_in() const;
	};

	[[nodiscard]] bool is_valid_ipv4(const std::string& ip);

	[[nodiscard]] bool is_valid_port(const int port_no);

} // namespace lynx::net