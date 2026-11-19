#pragma once

/*********************************************************
 *  ____     ___     ____   _  __  _____   _____      	 *
 * / ___|   / _ \   / ___| | |/ / | ____| |_   _|     	 *
 * \___ \  | | | | | |     | ' /  |  _|     | |       	 *
 *  ___) | | |_| | | |___  | . \  | |___    | |       	 *
 * |____/   \___/   \____| |_|\_\ |_____|   |_|       	 *
 *                                                       *
 * socket owns an operating-system socket descriptor.    *
 * It closes the descriptor automatically and transfers  *
 * ownership when moved; copying is not allowed.         *
 *********************************************************/

#include "net/socket_address.hpp"
#include "net/socket_config.hpp"
#include "net/socket_type.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <type_traits>
#include <unistd.h>
#include <utility>

namespace net
{
	using sockfd_t = int;
	inline constexpr sockfd_t invalid_socket_fd{-1};

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
	 * Describes a failed socket operation, including the  *
	 * system error code and operation category.            *
	 ********************************************************/
	struct socket_error
	{
		/****************************************************
		 * System error raised by the operating system.     *
		 ****************************************************/
		std::error_code code;

		/****************************************************
		 * Socket operation that produced the error.        *
		 ****************************************************/
		socket_operation operation;
	};

	/********************************************************
	 * Captures the current errno value for the operation.  *
	 ********************************************************/
	[[nodiscard]] inline std::unexpected<socket_error> make_error(socket_operation op) noexcept
	{
		return std::unexpected{socket_error{std::error_code{errno, std::system_category()}, op}};
	}

	/********************************************************
	 * Owns and manages the lifetime of a socket descriptor.*
	 ********************************************************/
	class socket
	{
	private:
		/****************************************************
		 * Native descriptor, or invalid_socket_fd when no  *
		 * descriptor is owned.                             *
		 ****************************************************/
		sockfd_t sock_fd{invalid_socket_fd};

		/****************************************************
		 * Takes ownership of a descriptor returned by      *
		 * accept().                                        *
		 ****************************************************/
		explicit socket(int fd) noexcept : sock_fd(static_cast<sockfd_t>(fd)) {};

		/****************************************************
		 * Closes the current descriptor and optionally     *
		 * replaces it with new_fd.                         *
		 ****************************************************/
		void reset(sockfd_t new_fd = invalid_socket_fd) noexcept;

	public:
		/****************************************************
		 * Closes the descriptor owned by this object.      *
		 ****************************************************/
		~socket();

		/****************************************************
		 * Copying is forbidden because ownership must      *
		 * remain unique.                                   *
		 ****************************************************/
		socket(const socket&) = delete;
		socket& operator=(const socket&) = delete;

		/****************************************************
		 * Transfers ownership and leaves the source object *
		 * without a descriptor.                            *
		 ****************************************************/
		socket(socket&& other) noexcept : sock_fd(std::exchange(other.sock_fd, invalid_socket_fd))
		{
		}

		/****************************************************
		 * Closes this object's descriptor, then takes      *
		 * ownership from the source object.                *
		 ****************************************************/
		socket& operator=(socket&& other) noexcept
		{
			if (this != &other)
			{
				reset(std::exchange(other.sock_fd, invalid_socket_fd));
			}
			return *this;
		};

		/********************************************************
		 * Creates a socket from the supplied configuration.    *
		 ********************************************************/
		[[nodiscard]] static std::expected<socket, socket_error> create(const socket_config& cfg) noexcept;

		/********************************************************
		 * Reports whether this object owns a valid descriptor. *
		 ********************************************************/
		[[nodiscard]] bool valid() const noexcept
		{
			return sock_fd >= 0;
		}

		/********************************************************
		 * Assigns the socket to the specified local address.   *
		 ********************************************************/
		[[nodiscard]] std::expected<void, socket_error> bind(const socket_address& sock_addr) noexcept;

		/********************************************************
		 * Enables incoming connections using the requested     *
		 * pending-connection backlog.                          *
		 ********************************************************/
		[[nodiscard]] std::expected<void, socket_error> listen(int backlog = SOMAXCONN) noexcept;

		/********************************************************
		 * Accepts one connection and returns a socket that     *
		 * owns the accepted descriptor.                        *
		 ********************************************************/
		[[nodiscard]] std::expected<socket, socket_error> accept() noexcept;

		/********************************************************
		 * Sends data and reports the number of bytes sent.     *
		 ********************************************************/
		[[nodiscard]] std::expected<size_t, socket_error> send(std::span<const std::byte> data);

		/********************************************************
		 * Reads available data and reports the number of bytes *
		 * received.                                            *
		 ********************************************************/
		[[nodiscard]] std::expected<size_t, socket_error> receive(std::span<std::byte> buffer);
	};

	static_assert(!std::is_copy_constructible_v<socket>);
} // namespace net