//
// This file is part of the Max-Net Project
//
// Copyright (c) 2019, Jonas Ohland
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

/*

#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <valijson/validator.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <json.hpp>

namespace ohlano {
    
    static nlohmann::json input_json = "{\"StationID\":0,\"InstanceID\":0,\"World\":{\"x\":0.0,\"y\":0.0,\"z\":0.0}}"_json;
    static valijson::adapters::NlohmannJsonAdapter input_json_adapter{ input_json };
    
    class validator_set {
    public:
        validator_set() : validator(valijson::Validator::TypeCheckingMode::kStrongTypes)
        { parser.populateSchema(input_json_adapter, schema); }
        valijson::Validator validator;
        valijson::Schema schema;
        valijson::SchemaParser parser;
    };

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
            
            return true;
		}

		bool deserialize() {

			try {
				json_ = nlohmann::json::parse(data_);
				return true;
			}
			catch (std::exception ex) {
				DBG("exception: ", ex.what());
				return false;
			}
			
		}
        
        bool validate(valijson::Validator& validator, valijson::Schema& schema,
                      valijson::ValidationResults* result_ptr){
            valijson::adapters::NlohmannJsonAdapter adapter{json_};
            return validator.validate(schema, adapter, result_ptr);
            
        }

		c74::min::atoms get_atoms() {
            
            c74::min::atoms output;
            
            if(json_["StationID"].is_number_integer()){
                output.push_back(json_["StationID"].get<int>());
            }
            
            if(json_["InstanceID"].is_number_integer()){
                output.push_back(json_["InstanceID"].get<int>());
            }
            
            if(json_["World"].is_object()){
                
                auto world = json_["World"];
                
                if(world["x"].is_number_float())
                    output.push_back(world["x"].get<float>());
                if(world["y"].is_number_float())
                    output.push_back(world["y"].get<float>());
                if(world["z"].is_number_float())
                    output.push_back(world["z"].get<float>());
                
            }
            
            return output;
			
		}

		void put_atoms(c74::min::atoms atms) {

		}
        
        void refcount_set(int value) const noexcept {
            int is = send_prog_refc.exchange(value);
            assert(is == 0);
        }

		int refcount_get() const noexcept {
			return send_prog_refc.load();
		}
        
        void notify_send() const noexcept{
            send_prog_refc++;
        }
        
        bool notify_send_done() const noexcept{
            return (--send_prog_refc == 0);
        }




	private:
        
		mutable std::atomic_int send_prog_refc{0};
        std::string data_;
        nlohmann::json json_;

	};

}
 */
