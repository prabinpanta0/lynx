#pragma once

/*********************************************************
 *  ____     ___     ____   _  __  _____   _____      	 *
 * / ___|   / _ \   / ___| | |/ / | ____| |_   _|     	 *
 * \___ \  | | | | | |     | ' /  |  _|     | |       	 *
 *  ___) | | |_| | | |___  | . \  | |___    | |       	 *
 * |____/   \___/   \____| |_|\_\ |_____|   |_|       	 *
 *                                                       *
 * socket is an RAII abstraction over platform sockets.  *
 * It provides a platform-independent API and is         *
 * move-only, with copying explicitly disabled.          *
 *********************************************************/

#include "net/socket_address.hpp"
#include "net/socket_config.hpp"
#include "net/socket_type.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <expected>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

namespace net
{
	using sockfd_t = int;
	inline constexpr sockfd_t INVALID_SOCKET_VAL{-1};

	enum class socket_operation
	{
		create,
		bind,
		listen,
		accept,
		send,
		receive
	};

	struct socket_error
	{
		std::error_code code;
		socket_operation operation;
	};

	/// A move-only RAII wrapper around a platform socket descriptor.
	class socket
	{
	private:
		// Default Initialize with invalid state
		socket_config sock_cfg{};
		sockfd_t sock_fd{-1};

		// Private constructor, used only by accept
		socket(int fd, socket_config cfg);

	public:
		/// Closes the owned socket descriptor, if any.
		~socket();

		/// Creates a socket using the supplied domain, type, and protocol.
		socket(socket_config cfg);

		socket(const socket&) = delete;
		socket& operator=(const socket&) = delete;

		socket(socket&& other) noexcept;
		socket& operator=(socket&& other) noexcept;

		/// Associates the socket descriptor with the given local address.
		[[nodiscard]] std::expected<void, socket_error> bind(const socket_address& sock_addr);

		/// Marks the socket as passive and enables incoming connections.
		[[nodiscard]] std::expected<void, socket_error> listen(int backlog = SOMAXCONN);

		/// Accepts one incoming connection and returns its dedicated socket.
		[[nodiscard]] std::expected<socket, socket_error> accept();

		/// Sends all bytes in data and returns the number of bytes sent.
		[[nodiscard]] std::expected<ssize_t, socket_error> send(std::span<const std::byte> data);

		/// Reads available bytes into buffer and returns the bytes received.
		[[nodiscard]] std::expected<ssize_t, socket_error> receive(std::span<std::byte> buffer);
	};
} // namespace net