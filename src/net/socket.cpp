#include "net/socket.hpp"
#include "net/net_except.hpp"
#include "net/socket_address.hpp"
#include "net/socket_config.hpp"
#include <cstddef>
#include <expected>
#include <iostream>
#include <span>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace net
{

	std::expected<socket, socket_error> socket::create(socket_config cfg)
	{

	};



	// A private descriptor-taking constructor is used only by accept().
	// It transfers an already-created OS socket into this socket instance
	// while preserving the configuration of the listening socket.
	socket::socket(int fd) : sock_fd{fd}
	{
	}

	// RAII cleanup closes only valid descriptors. Descriptor ownership is
	// transferred by move operations, which reset the moved-from descriptor
	// to an invalid value so that its destructor does not close it again.
	socket::~socket()
	{
		if (sock_fd >= 0)
		{
			::close(sock_fd);
		}
	}

	// Move construction transfers descriptor ownership out of other.
	// The source is reset to -1 after the transfer so only this object closes
	// the descriptor in its destructor.
	socket::socket(socket&& other) noexcept : sock_fd{ std::exchange(other.sock_fd, INVALID_SOCKET_VAL )}
	{
	}

	// Move assignment closes the current descriptor before taking ownership
	// of another socket. Self-move assignment is skipped because closing or
	// resetting the existing object would destroy valid state.
	socket& socket::operator=(socket&& other) noexcept
	{
		if (this != &other)
		{
			if (sock_fd >= 0)
			{
				close(sock_fd);
			}
			sock_fd = other.sock_fd;
			other.sock_fd = -1;
		}
		return *this;
	}

	// Convert the address to the native IPv4 representation, then associate
	// it with this socket through bind(). A failure includes the requested
	// address in net_except so the caller can identify the binding target.
	std::expected<void, socket_error> socket::bind(const socket_address& sock_addr)
	{
		sockaddr_in addr{sock_addr.to_sockaddr_in()};

		if (::bind(sock_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		{
		}
	}

	// Listen switches the socket from active use to passive server mode.
	// backlog is the maximum pending connection queue length; SOMAXCONN asks
	// the platform to use a recommended large queue when no custom size is
	// supplied.
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

	// accept() blocks until a connection arrives, then creates a separate
	// socket from the returned descriptor. The returned socket owns the
	// connected descriptor and does not share or invalidate the listening
	// socket.
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

	// send() can transfer only part of a span in one OS call. This loop
	// advances through the remaining bytes until every byte is sent.
	// EINTR is treated as a retryable interruption, while any other negative
	// result is an OS error and is reported with the number of bytes already
	// transferred.
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

	// receive() performs one blocking read into the caller-provided buffer.
	// An empty span requires no OS call and returns 0 immediately. EINTR
	// retries the operation recursively; a positive result is the number of
	// bytes written into the buffer, and 0 can indicate a closed connection.
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
} // namespace net