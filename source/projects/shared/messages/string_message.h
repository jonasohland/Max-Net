#pragma once

#include <string>
#include <boost/asio/buffer.hpp>


namespace ohlano {
	class string_message {
		std::string mess;
	public:

		string_message(std::string&& mess_in) {
			mess = std::move(mess_in);
		}

		string_message(const std::string& mess_in) : mess(mess_in) {}

		string_message(string_message&& other) : mess(std::move(other.mess)) {}

		string_message(const string_message& other) : mess(other.mess) {}

		string_message* operator=(const string_message& other) {
			mess = other.mess;
            return this;
		}

		template<typename ConstBufferSequence>
		static string_message from_const_buffer(ConstBufferSequence const& seq) {

			std::string buf;
			buf.reserve(boost::asio::buffer_size(seq));

			for (auto buffer : boost::beast::detail::buffers_range(seq)) {
				buf.append(static_cast<char const*>(buffer.data()), buffer.size());
			}

			return string_message(buf);
		}

		std::string str() const {
			return mess;
		}

		operator std::string() {
			return mess;
		}

		const std::string::value_type* data() const {
			return mess.data();
		}

		const size_t size() const {
			return mess.size();
		}
	};
}
