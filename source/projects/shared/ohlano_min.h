#pragma once

#include <functional>
#include <string>
#include <sstream>


namespace ohlano {

	class console_stream_adapter {
	public:

		class endl_type {};
		typedef std::function<void(std::string)> handler_function_type;

		console_stream_adapter(handler_function_type handler, bool _space_separated = false, std::string _separator_char = std::string("")) {
			separator_char = _separator_char;
			space_separator = _space_separated;
			string_handler = handler;
		}

		console_stream_adapter(const console_stream_adapter& other) {
			string_handler = other.string_handler;
			separator_char = other.separator_char;
			space_separator = other.space_separator;
		}

		bool has_sep_char() {
			return separator_char.size() > 0;
		}

		bool space_separated() {
			return space_separator;
		}

		template<typename T>
		console_stream_adapter& operator<<(T&& input) {

			sstr << std::forward<T>(input);
			
			if (space_separated()) 
				sstr << " ";
			

			if (has_sep_char()) {
				sstr << separator_char;
				if (space_separated())
					sstr << " ";
			}
	
			return *this;
		}

		console_stream_adapter& operator<<(endl_type x) {

			std::string out = sstr.str();

			if (space_separated()) { out.pop_back(); }
			if (has_sep_char()) { out.pop_back(); }

			string_handler(out);

			sstr.str(std::string());
			return *this;
		}

	private:


		handler_function_type string_handler;
		console_stream_adapter() = delete;

		std::stringstream sstr;
		std::string separator_char;
		bool space_separator;
	};

	static console_stream_adapter::endl_type endl;
}