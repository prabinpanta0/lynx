#pragma once

/*********************************************************
 *  ____     ___     ____   _  __  _____   _____      	 *
 * / ___|   / _ \   / ___| | |/ / | ____| |_   _|     	 *
 * \___ \  | | | | | |     | ' /  |  _|     | |       	 *
 *  ___) | | |_| | | |___  | . \  | |___    | |       	 *
 * |____/   \___/   \____| |_|\_\ |_____|   |_|       	 *
 *                                                       *
 * Defines socket errors and native socket configuration *
 * types used by the Lynx networking implementation.     *
 *********************************************************/

#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>

namespace lynx::net
{
	using socket_handle = int;

	/********************************************************
	 * Identifies the socket operation associated with an   *
	 * error.                                               *
	 ********************************************************/
	enum class socket_operation : uint8_t
	{
		create,
		bind,
		listen,
		accept,
		send,
		receive
	};

	/********************************************************
	 * Describes a failed socket operation, including the   *
	 * system error code and operation category.            *
	 ********************************************************/
	struct socket_error
	{
		std::error_code code;
		socket_operation operation;
	};

	/********************************************************
	 * Defines the address family passed to the socket API. *
	 ********************************************************/
	enum class socket_domain : std::uint8_t
	{
		IPv4 = AF_INET,
		IPv6 = AF_INET6,
		UNIX = AF_UNIX,
		UNSPECIFIED = AF_UNSPEC
	};

	/********************************************************
	 * Defines the communication semantics used by a socket.*
	 ********************************************************/
	enum class socket_type : std::uint8_t
	{
		STREAM = SOCK_STREAM,
		DATAGRAM = SOCK_DGRAM,
		RAW = SOCK_RAW
	};

	/********************************************************
	 * Defines the transport protocol passed to the socket  *
	 * API, or requests the protocol selected by the system.*
	 ********************************************************/
	enum class socket_protocol : std::uint8_t
	{
		DEFAULT = 0,
		TCP = IPPROTO_TCP,
		UDP = IPPROTO_UDP
	};
} // namespace lynx::net