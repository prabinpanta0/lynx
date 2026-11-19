#pragma once
#include "net/socket_address.hpp"
#include "net/socket_type.hpp"
namespace net
{
	/*
	 * Defaults to an IPv4 TCP stream socket bound to 0.0.0.0:8080.
	 * The protocol remains at its default value so the native socket API
	 * selects the protocol appropriate for the configured type.
	 */
	struct socket_config
	{
		socket_domain domain{socket_domain::IPv4};
		socket_type type{socket_type::Sstream};
		socket_protocol protocol{socket_protocol::Default};
		socket_address sock_addr{.ip = "0.0.0.0", .port = 8080};
	};
} // namespace net