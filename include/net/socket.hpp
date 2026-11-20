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
#include <expected>
#include <span>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <type_traits>
#include <unistd.h>
#include <utility>

namespace lynx::net
{
	inline constexpr socket_handle invalid_socket_fd{-1};

	[[nodiscard]] inline std::unexpected<socket_error> make_error(socket_operation operation, int error) noexcept
	{
		return std::unexpected{socket_error{std::error_code{error, std::generic_category()}, operation}};
	}

	class socket
	{
	private:
		/****************************************************
		 * Native descriptor, or invalid_socket_fd when no  *
		 * descriptor is owned.                             *
		 ****************************************************/
		socket_handle sock_fd{invalid_socket_fd};

		/****************************************************
		 * Takes ownership of a descriptor returned by      *
		 * accept().                                        *
		 ****************************************************/
		explicit socket(int fd) noexcept : sock_fd(static_cast<socket_handle>(fd)) {};

		void reset(const socket_handle new_fd = invalid_socket_fd) noexcept;

	public:
		~socket();

		/*
		 * Transfers ownership and leaves the source object 
		 * with an invalid descriptor.                   
		 */
		socket(socket&& other) noexcept : sock_fd(std::exchange(other.sock_fd, invalid_socket_fd)){}

		/*
		 * Closes this object's descriptor if valid, then takes      
		 * ownership from the source object and assigns the source
		 * descriptor with an invalid one.                
		 */
		socket& operator=(socket&& other) noexcept
		{
			if (this != &other)
			{
				reset(std::exchange(other.sock_fd, invalid_socket_fd));
			}
			return *this;
		};

		[[nodiscard]] static std::expected<socket, socket_error> create(const socket_config& config) noexcept;

		[[nodiscard]] std::expected<void, socket_error> bind(const socket_address& sock_addr) noexcept;

		[[nodiscard]] std::expected<void, socket_error> listen(int backlog = SOMAXCONN) noexcept;

		[[nodiscard]] std::expected<socket, socket_error> accept() noexcept;

		[[nodiscard]] std::expected<size_t, socket_error> send(std::span<const std::byte> data);

		[[nodiscard]] std::expected<size_t, socket_error> receive(std::span<std::byte> buffer);

		/********************************************************
		 * Reports whether this object owns a valid descriptor. *
		 ********************************************************/
		[[nodiscard]] bool valid() const noexcept
		{
			return sock_fd >= 0;
		}
	};

	static_assert(!std::is_copy_constructible_v<socket>);
} // namespace lynx::net