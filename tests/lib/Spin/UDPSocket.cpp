#include "UDPSocket.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <Spin/UDPSocket.h>
#include <Spin/Handlers/UDPDataHandler.h>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#	include <Windows.h>
#	define yield() Sleep(1)
#else
extern "C" {
#	include <pthread.h>
}
#	define yield pthread_yield
#endif

namespace Tests
{
	namespace Spin
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(UDPSocket);

		void UDPSocket::setUp()
		{ /* no-op */ }

		void UDPSocket::tearDown()
		{ /* no-op */ }

		void UDPSocket::tryCreateInstance01()
		{
			::Spin::UDPSocket udp_socket(::Spin::Details::Address(0), 0);
		}

		void UDPSocket::tryCreateInstance02()
		{
			::Spin::UDPSocket udp_socket(::Spin::Details::Address(0), 9005);
		}

		void UDPSocket::tryComm01()
		{
			::Spin::UDPSocket udp_recv_socket(::Spin::Details::Address(0), 9005);
			::Spin::UDPSocket udp_send_socket(::Spin::Details::Address(0), 0);
			udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9005, "test");
			std::vector< char > buffer;
			::Spin::Details::Address remote_address(0);
			unsigned short remote_port;
			std::size_t received;
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.recv(buffer);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(127, 0, 0, 1));
			// we know nothing about the port: it's random
			CPPUNIT_ASSERT(received == 4);
			CPPUNIT_ASSERT(buffer[0] == 't');
			CPPUNIT_ASSERT(buffer[1] == 'e');
			CPPUNIT_ASSERT(buffer[2] == 's');
			CPPUNIT_ASSERT(buffer[3] == 't');
		}

		void UDPSocket::tryComm02()
		{
			::Spin::UDPSocket udp_recv_socket(::Spin::Details::Address(0), 9005);
			CPPUNIT_ASSERT(!udp_recv_socket.poll());
			::Spin::UDPSocket udp_send_socket(::Spin::Details::Address(0), 0);
			udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9005, "test");
			std::vector< char > buffer;
			::Spin::Details::Address remote_address(0);
			unsigned short remote_port;
			int i(0);
			bool poll_returned_true(false);
			while (!(poll_returned_true = udp_recv_socket.poll()) && i++ < 500) yield();
			CPPUNIT_ASSERT(poll_returned_true);
			std::size_t received;
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.recv(buffer);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(127, 0, 0, 1));
			// we know nothing about the port: it's random
			CPPUNIT_ASSERT(received == 4);
			CPPUNIT_ASSERT(buffer[0] == 't');
			CPPUNIT_ASSERT(buffer[1] == 'e');
			CPPUNIT_ASSERT(buffer[2] == 's');
			CPPUNIT_ASSERT(buffer[3] == 't');
		}

		void UDPSocket::tryComm03()
		{
			struct Handler : ::Spin::Handlers::UDPDataHandler
			{
				Handler()
					: received_(false),
					  remote_address_(0)
				{ /* no-op */ }

				/*virtual */void onDataReady(::Spin::UDPSocket * socket) const/* = 0;*/
				{
					std::vector< char > buffer;
					boost::tuple< ::Spin::Details::Address, unsigned short, size_t > rv(socket->recv(buffer));
					received_ = true;
					buffer_ = buffer;
					remote_address_ = boost::tuples::get<0>(rv);
					remote_port_ = boost::tuples::get<1>(rv);
					buffer_size_ = boost::tuples::get<2>(rv);
				}

				mutable bool received_;
				mutable std::vector< char > buffer_;
				mutable ::Spin::Details::Address remote_address_;
				mutable unsigned short remote_port_;
				mutable size_t buffer_size_;
			};

			Handler data_handler;

			::Spin::UDPSocket udp_recv_socket(::Spin::Details::Address(0), 9005);
			CPPUNIT_ASSERT(!udp_recv_socket.poll());
			udp_recv_socket.setDataHandler(data_handler);

			::Spin::UDPSocket udp_send_socket(::Spin::Details::Address(0), 0);
			udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9005, "test");

			int i(0);
			while (!data_handler.received_ && i++ < 500) yield();

			CPPUNIT_ASSERT(data_handler.received_);
			CPPUNIT_ASSERT(data_handler.remote_address_ == ::Spin::Details::Address(127, 0, 0, 1));
			// we know nothing about the port: it's random
			CPPUNIT_ASSERT(data_handler.buffer_size_ == 4);
			CPPUNIT_ASSERT(data_handler.buffer_size_ == data_handler.buffer_.size());
			CPPUNIT_ASSERT(data_handler.buffer_[0] == 't');
			CPPUNIT_ASSERT(data_handler.buffer_[1] == 'e');
			CPPUNIT_ASSERT(data_handler.buffer_[2] == 's');
			CPPUNIT_ASSERT(data_handler.buffer_[3] == 't');
		}

		void UDPSocket::tryPeek01()
		{
			::Spin::UDPSocket udp_recv_socket(::Spin::Details::Address(0), 9005);
			::Spin::UDPSocket udp_send_socket(::Spin::Details::Address(0), 0);
			udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9005, "test");
			std::vector< char > buffer;
			::Spin::Details::Address remote_address(0);
			unsigned short remote_port;
			std::size_t received;
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.peek(buffer);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(127, 0, 0, 1));
			// we know nothing about the port: it's random
			CPPUNIT_ASSERT(received == 4);
			unsigned short port = remote_port;
			CPPUNIT_ASSERT(buffer[0] == 't');
			CPPUNIT_ASSERT(buffer[1] == 'e');
			CPPUNIT_ASSERT(buffer[2] == 's');
			CPPUNIT_ASSERT(buffer[3] == 't');
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.peek(buffer);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(127, 0, 0, 1));
			CPPUNIT_ASSERT(remote_port == port);
			CPPUNIT_ASSERT(received == 4);
			CPPUNIT_ASSERT(buffer[0] == 't');
			CPPUNIT_ASSERT(buffer[1] == 'e');
			CPPUNIT_ASSERT(buffer[2] == 's');
			CPPUNIT_ASSERT(buffer[3] == 't');
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.peek(buffer);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(127, 0, 0, 1));
			CPPUNIT_ASSERT(remote_port == port);
			CPPUNIT_ASSERT(received == 4);
			CPPUNIT_ASSERT(buffer[0] == 't');
			CPPUNIT_ASSERT(buffer[1] == 'e');
			CPPUNIT_ASSERT(buffer[2] == 's');
			CPPUNIT_ASSERT(buffer[3] == 't');
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.recv(buffer);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(127, 0, 0, 1));
			CPPUNIT_ASSERT(remote_port == port);
			CPPUNIT_ASSERT(received == 4);
			CPPUNIT_ASSERT(buffer[0] == 't');
			CPPUNIT_ASSERT(buffer[1] == 'e');
			CPPUNIT_ASSERT(buffer[2] == 's');
			CPPUNIT_ASSERT(buffer[3] == 't');
		}

		void UDPSocket::tryTimeOut01()
		{
			::Spin::UDPSocket udp_recv_socket(::Spin::Details::Address(0), 9005);
			std::vector< char > buffer;
			::Spin::Details::Address remote_address(0);
			unsigned short remote_port;
			std::size_t received;
			boost::tie(remote_address, remote_port, received) = udp_recv_socket.recv(buffer, 500);
			CPPUNIT_ASSERT(remote_address == ::Spin::Details::Address(0, 0, 0, 0));
			// we know nothing about the port: it's random
			CPPUNIT_ASSERT(received == 0);
		}

		void UDPSocket::regression01()
		{
			struct Handler
			{
				Handler(::Spin::UDPSocket & socket)
					: socket_(&socket)
					, thread_(0)
					, is_closing_(false)
					, received_(false)
				{
					thread_ = new boost::thread(boost::bind(&Handler::thread, this));
				}

				~Handler()
				{
					socket_->close();
					is_closing_ = true;
					thread_->join();
					delete thread_;
				}

				void thread()
				{
					while (!is_closing_)
					{
						::Spin::Details::Address address;
						unsigned short port;
						std::size_t size;
						std::vector< char > buffer;

						boost::tie(address, port, size) = socket_->recv(buffer);
						received_ = true;
					}
				}

				::Spin::UDPSocket * socket_;
				boost::thread * thread_;
				bool is_closing_;
				bool received_;
			};


			::Spin::UDPSocket udp_send_socket(::Spin::Details::Address(127, 0, 0, 1), 9721);
			Handler send_socket_data_handler(udp_send_socket);

			{
				::Spin::UDPSocket udp_recv_socket(::Spin::Details::Address(127, 0, 0, 1), 9722);
				Handler recv_socket_data_handler(udp_recv_socket);
				udp_recv_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9721, "hello");
				while (!send_socket_data_handler.received_);
				send_socket_data_handler.received_ = false;
				udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9722, "hello");
				while (!recv_socket_data_handler.received_);
				recv_socket_data_handler.received_ = false;
			}
			udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9722, "hello");
			Sleep(1000);
			udp_send_socket.send(::Spin::Details::Address(127, 0, 0, 1), 9722, "hello");
		}
	}
}
