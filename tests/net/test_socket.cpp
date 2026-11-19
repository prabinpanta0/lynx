#include <catch2/catch_test_macros.hpp>

#include "net/net_except.hpp"
#include "net/socket.hpp"
#include "net/socket_address.hpp"
#include "net/socket_config.hpp"

#include <cstddef>
#include <netinet/in.h>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std::literals;

TEST_CASE("socket lifecycle", "[socket]")
{
	net::socket_config cfg;

	SECTION("construction")
	{
		net::socket s(cfg);
	}

	SECTION("move constructor")
	{
		net::socket a(cfg);
		net::socket b(std::move(a));
	}

	SECTION("move assignment")
	{
		net::socket a(cfg);
		net::socket b(cfg);
		b = std::move(a);
	}

	SECTION("self move assignment")
	{
		net::socket s(cfg);
		s = std::move(s);
	}
}

TEST_CASE("socket bind", "[socket][bind]")
{
	net::socket_config cfg;

	SECTION("valid bind")
	{
		net::socket s(cfg);
		net::Socket_address addr{"127.0.0.1", 9300};

		s.bind(addr);
	}

	SECTION("invalid ip")
	{
		net::socket s(cfg);
		net::Socket_address addr{"999.999.999.999", 0};

		CHECK_THROWS_AS(s.bind(addr), net::net_except);
	}

	SECTION("port already in use")
	{
		net::Socket_address addr{"127.0.0.1", 9301};

		net::socket s1(cfg);
		net::socket s2(cfg);

		s1.bind(addr);

		CHECK_THROWS_AS(s2.bind(addr), net::net_except);
	}
}

TEST_CASE("socket listen", "[socket][listen]")
{
	net::socket_config cfg;

	SECTION("listen after bind")
	{
		net::socket s(cfg);
		net::Socket_address addr{"127.0.0.1", 9302};

		s.bind(addr);
		s.listen();
	}
}

TEST_CASE("socket accept", "[socket][accept]")
{
	net::socket_config cfg;
	net::Socket_address addr{"127.0.0.1", 9303};

	net::socket server(cfg);

	server.bind(addr);
	server.listen();

	std::thread client(
		[&]
		{
			int raw = ::socket(AF_INET, SOCK_STREAM, 0);

			sockaddr_in native = addr.to_sockaddr_in();

			::connect(raw, reinterpret_cast<sockaddr*>(&native), sizeof(native));

			::close(raw);
		});

	net::socket conn = server.accept();

	client.join();
}

TEST_CASE("socket send receive", "[socket][io]")
{
	net::socket_config cfg;
	net::Socket_address addr{"127.0.0.1", 9304};

	net::socket server(cfg);

	server.bind(addr);
	server.listen();

	std::string_view msg = "Hello Lynx!";
	std::string received;

	std::thread client(
		[&]
		{
			int raw = ::socket(AF_INET, SOCK_STREAM, 0);

			sockaddr_in native = addr.to_sockaddr_in();

			::connect(raw, reinterpret_cast<sockaddr*>(&native), sizeof(native));

			char buf[128]{};

			ssize_t n = ::recv(raw, buf, sizeof(buf), 0);

			if (n > 0)
				received.assign(buf, n);

			::close(raw);
		});

	net::socket conn = server.accept();

	auto bytes = std::as_bytes(std::span(msg));

	ssize_t sent = conn.send(bytes);

	REQUIRE(sent == msg.size());

	client.join();

	REQUIRE(received == msg);
}

TEST_CASE("socket large payload", "[socket][large]")
{
	net::socket_config cfg;
	net::Socket_address addr{"127.0.0.1", 9305};

	net::socket server(cfg);

	server.bind(addr);
	server.listen();

	std::vector<std::byte> payload(1 << 20);

	std::thread client(
		[&]
		{
			int raw = ::socket(AF_INET, SOCK_STREAM, 0);

			sockaddr_in native = addr.to_sockaddr_in();

			::connect(raw, reinterpret_cast<sockaddr*>(&native), sizeof(native));

			std::vector<std::byte> buffer(payload.size());

			size_t total = 0;

			while (total < buffer.size())
			{
				ssize_t r = ::recv(raw, reinterpret_cast<char*>(buffer.data()) + total, buffer.size() - total, 0);

				if (r <= 0)
					break;

				total += r;
			}

			REQUIRE(total == payload.size());

			::close(raw);
		});

	net::socket conn = server.accept();

	ssize_t sent = conn.send(payload);

	REQUIRE(sent == payload.size());

	client.join();
}