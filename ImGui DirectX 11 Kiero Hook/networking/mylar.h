#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <cstdint>
#include <thread>
#include <string>
#include <mutex>
#include <vector>

namespace mylar
{

	namespace file
	{

		constexpr auto buffer_size = 4096;

		static int64_t get_file_size(const char* path)
		{

			FILE* f;
			if (fopen_s(&f, path, "rb") != 0) {
				return -1;
			}
			_fseeki64(f, 0, SEEK_END);
			const int64_t len = _ftelli64(f);
			fclose(f);
			return len;

		}

	}

	namespace constants
	{

		enum _client_header : int16_t
		{

			HEADER = 0xBC4D,

		};

		enum _request_id : int16_t
		{

			LOGIN_REQUEST,
			LOGIN_RESPONSE,
			PING_REQUEST,
			PING_RESPONSE,
			DOWNLOAD_REQUEST,
			DOWNLOAD_RESPONSE,
			REQUEST_ACK,
			INJECTION_NOTIFY,
			INJECTION_ACK,
			MODULE_GET_USER_INFO,
			MODULE_GET_USER_INFO_ACK,

			REQUEST_MAX,
			REQUEST_DEFAULT = 23

		};

		enum _download_id : int16_t
		{

			DOWNLOAD_VULERNABLE_DRIVER = 1,
			DOWNLOAD_UNSIGNED_DRIVER,
			DOWNLOAD_MAPPER_MODULE,

			DOWNLOAD_MAX,

		};

		enum _subscription_id : int16_t
		{

			DEFAULT = 1,
			UNTURNED,
			RUST,
			COMBAT_MASTER_FREE,
			REDMATCH_2,
			UNTURNED_SPOOFER,

		};

	}

	namespace network
	{

		struct packet_t
		{

			constants::_request_id m_type{ constants::_request_id::REQUEST_DEFAULT };
			char m_username[64] = { NULL };
			char m_password[64] = { NULL };
			char m_error_message[32] = { NULL };
			ULONGLONG m_client_heartbeat_time{ NULL };
			uint32_t m_serial_number{ NULL };
			int m_protected{ 0xFFFF };
			int m_debugged{ 0xFFFF };
			int m_virtual_machine{ 0xFFFF };
			int m_code_section{ 0xFFFF };
			int m_checksum{ NULL };
			int m_ping{ NULL };
			constants::_client_header m_header{ constants::_client_header::HEADER };
			constants::_download_id m_download_id{ constants::_download_id::DOWNLOAD_MAX };
			constants::_subscription_id m_subscription_id{ constants::_subscription_id::DEFAULT };
			bool m_successful_login{ false };

		};

		struct remote_sub_t
		{

			char m_name[32] = { NULL };
			char m_expiry[128] = { NULL };
			constants::_subscription_id m_sub_id{ constants::_subscription_id::DEFAULT };

		};

		struct remote_file_t
		{

			int64_t size{ NULL };
			std::vector<uint8_t> contents{};

		};

	}

	struct local_client_t
	{

	private:

		SOCKET m_socket{};
		WSADATA m_socket_data{};
		sockaddr_in m_socket_addr{};

		ULONGLONG m_encryption_key{ NULL };

		bool m_connected{ false };

	public:

		~local_client_t() { destroy(); }

		void destroy()
		{

			if (m_socket)
				closesocket(m_socket);

			WSACleanup();

		}

		DECLSPEC_NOINLINE bool create()
		{
#ifndef NO_SECURITY
			VIRTUALIZER_DOLPHIN_RED_START;
#endif

			const auto version = 0x101;

			if (WSAStartup(version, &m_socket_data))
				return false;

			m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

			if (m_socket == INVALID_SOCKET)
				return false;

			m_socket_addr.sin_addr.S_un.S_addr = 2247440386/*16777343*/;
			m_socket_addr.sin_family = AF_INET;
			m_socket_addr.sin_port = htons(0x80);

			m_connected = (connect(m_socket, (sockaddr*)&m_socket_addr, sizeof(m_socket_addr)) == NULL);

			if (!m_connected)
				return false;

			if (!in(m_encryption_key))
				return false;

			network::packet_t ack_request{ constants::REQUEST_ACK };
			ack_request.m_client_heartbeat_time = GetTickCount64();

			if (!out(ack_request))
				return false;

#ifndef NO_SECURITY
			VIRTUALIZER_DOLPHIN_RED_END;
#endif

			return m_connected;
		}

		template <typename type>
		DECLSPEC_NOINLINE bool out(type& data) const
		{
#ifndef NO_SECURITY
			VIRTUALIZER_DOLPHIN_RED_START;
#endif

			char* buf = reinterpret_cast<char*>(&data);

			for (int i = 0; i < sizeof(data); i++)
				buf[i] ^= static_cast<char>(m_encryption_key);

			int32_t result = ::send(m_socket, buf, (int)sizeof(data), 0);

#ifndef NO_SECURITY
			VIRTUALIZER_DOLPHIN_RED_END;
#endif

			return result != SOCKET_ERROR;

		}

		template <typename type>
		DECLSPEC_NOINLINE bool in(type& data) const
		{
#ifndef NO_SECURITY
			VIRTUALIZER_DOLPHIN_RED_START;
#endif

			auto buf = reinterpret_cast<char*>(&data);

			auto bytes_recieved = ::recv(m_socket, buf, (int)sizeof(data), 0);

			for (int i = 0; i < sizeof(data); i++)
				buf[i] ^= static_cast<char>(m_encryption_key);

#ifndef NO_SECURITY
			VIRTUALIZER_DOLPHIN_RED_END;
#endif

			return (bytes_recieved == sizeof(data));

		}

		DECLSPEC_NOINLINE network::remote_file_t get_file() const
		{

			int64_t file_size = 0;

			if (!in(file_size))
				return {};

			int64_t bytes_recieved = 0, total_bytes_recieved = 0;
			char buffer[file::buffer_size];

			network::remote_file_t remote_file{};
			remote_file.size = file_size;

			while (total_bytes_recieved < file_size)
			{

				bytes_recieved = ::recv(m_socket, buffer, file::buffer_size, 0);

				if (bytes_recieved == SOCKET_ERROR)
					return {};

				for (int i = 0; i < bytes_recieved; i++)
					remote_file.contents.push_back(buffer[i]);

				total_bytes_recieved += bytes_recieved;

			}

			return remote_file;

		}

		bool is_connected() const { return m_connected; }

	};

}