#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <json.hpp>


namespace ohlano {

	class json_message {

		class json_message_factory {
		public:
			json_message* allocate() {
				return new json_message();
			}

			void deallocate(const json_message* mess) {
				delete mess;
			}
		};

	public:

		using factory = json_message_factory;

		json_message() {
		}

		virtual ~json_message() {
		}

		template<typename ConstBufferSequence>
		static void from_const_buffers(ConstBufferSequence buffers, json_message* msg, bool text) {

			if (text) {
				msg->data_.reserve(boost::asio::buffer_size(buffers));

				for (const auto& buffer : boost::beast::detail::buffers_range(buffers)) {
					std::copy(boost::asio::buffers_begin(buffer), boost::asio::buffers_end(buffer), std::back_inserter(msg->data_));
				}
			}
			else {
				DBG("received binary data");
			}

		}

		std::string& vect() {
			return data_;
		}

		nlohmann::json& json() {
			return json_;
		}

		const nlohmann::json& json() const {
			return json_;
		}

		const std::string::value_type* data() const {
			return data_.data();
		}

		size_t size() const {
			return data_.size();
		}

		bool serialize() {
			data_ = std::string(json_.dump());

			DBG(json_.dump(4));
		}

		bool deserialize() {

			try {
				json_ = nlohmann::json::parse(data_);
				DBG("received: ", json_.dump(4));
				return true;
			}
			catch (std::exception ex) {
				DBG("exception: ", ex.what());
				return false;
			}
			
		}

		c74::min::atoms get_atoms() {

			auto world = json_.at("World");

			return {
				c74::min::atom(static_cast<int>(json_.at("StationID"))),
				c74::min::atom(static_cast<int>(json_.at("InstanceID"))),
				c74::min::atom(static_cast<double>(world.at("x"))),
				c74::min::atom(static_cast<double>(world.at("y"))),
				c74::min::atom(static_cast<double>(world.at("z")))
			};
		}

		void put_atoms(c74::min::atoms atms) {

		}




	private:
		std::string data_;
		nlohmann::json json_;

	};

}
