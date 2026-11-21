#include "net/socket.hpp"
#include "net/socket_address.hpp"
#include "net/socket_config.hpp"
#include "net/socket_type.hpp"

/**********************************************************
 *  ____   ___   ____ _  _______ _____                    *
 * / ___| / _ \ / ___| |/ / ____|_   _|                   *
 * \___ \| | | | |   | ' /|  _|   | |                     *
 *  ___) | |_| | |___| . \| |___  | |                     *
 * |____/ \___/ \____|_|\_\_____| |_|                     *
 *                                                        *
 * Socket descriptor implementation and operations.       *
 **********************************************************/

#include <cerrno>
#include <cstddef>
#include <expected>
#include <iostream>
#include <span>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace lynx::net
{

	/*
	 * Replaces the owned descriptor with "new_fd", closing the 
	 * previous descriptor if it is valid.                         
	 */
	void socket::reset(const socket_handle new_fd) noexcept
	{
		if (sock_fd >= 0)
		{
			::close(sock_fd);
		}
		sock_fd = new_fd;
	}

	/*
	 * Closes the owned descriptor through RAII cleanup.    
	 * Move operations invalidate the source descriptor.    
	 */
	socket::~socket()
	{
		reset();
	}

	/*
	 * Creates a socket using the supplied configuration.
	 */
	std::expected<socket, socket_error> socket::create(const socket_config& config) noexcept
	{
		const auto fd{::socket(static_cast<int>(config.domain),
							   static_cast<int>(config.type),
							   static_cast<int>(config.protocol))};
		if (static_cast<socket_handle>(fd) == invalid_socket_fd)
		{
			const int error{errno};
			return make_error(socket_operation::create, error);
		}
		return socket{fd};
	};

	/*
	 * Converts the address to native form and binds the     
	 * socket to that local endpoint.                        
	 */
	std::expected<void, socket_error> socket::bind(const socket_address& sock_addr) noexcept
	{
		const auto addr{sock_addr.to_sockaddr_in()};
		if (::bind(sock_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			const int error{errno};
			return make_error(socket_operation::bind, error);
		}
		return {};
	}

	/********************************************************
	 * Switches the socket into passive listening mode.      *
	 * Backlog controls the pending connection queue size.   *
	 ********************************************************/
	std::expected<void, socket_error> socket::listen(int backlog)
	{
		if (sock_fd < 0)
		{
			throw net_except("Cannot listen: socket uninitialized.");
		}

		if (::listen(sock_fd, backlog) < 0)
		{
			throw net_except("Failed to listen: ");
		}

		std::cout << "socket is now listening...\n";
	}

	/********************************************************
	 * Blocks until an incoming connection arrives, then     *
	 * creates a socket that owns the accepted descriptor.   *
	 ********************************************************/
	std::expected<socket, socket_error> socket::accept()
	{
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);

		int client_fd = ::accept(sock_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

		if (client_fd < 0)
		{
			throw net_except("Accept failed: ");
		}
	}

	/********************************************************
	 * Sends all remaining bytes in a loop. Interrupted      *
	 * writes are retried; other failures return an error.   *
	 ********************************************************/
	std::expected<ssize_t, socket_error> socket::send(std::span<const std::byte> data)
	{
		size_t total_sent = 0;
		while (total_sent < data.size())
		{
			auto remaining = data.subspan(total_sent);
			ssize_t sent = ::send(sock_fd, remaining.data(), remaining.size(), MSG_NOSIGNAL);

			if (sent < 0)
			{
				if (errno == EINTR)
					continue;

				throw net_except("Send failed after " + std::to_string(total_sent) + " bytes: ");
			}

			total_sent += static_cast<size_t>(sent);
		}

		return static_cast<int>(total_sent);
	}

	/********************************************************
	 * Performs one blocking receive operation. An empty     *
	 * buffer returns zero without calling the OS.           *
	 ********************************************************/
	std::expected<ssize_t, socket_error> socket::receive(std::span<std::byte> buffer)
	{
		if (buffer.empty())
			return 0;

		ssize_t bytes_read = ::recv(sock_fd, buffer.data(), buffer.size(), 0);

		if (bytes_read < 0)
		{
			if (errno == EINTR)
				return receive(buffer);

			throw net_except("Receive failed: ");
		}

		return static_cast<int>(bytes_read);
	}
} // namespace lynx::net