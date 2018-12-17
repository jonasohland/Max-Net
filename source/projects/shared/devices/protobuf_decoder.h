#pragma once

#include "../../../build/source/projects/websocketclient/iiwaPosition.pb.h"
#include "../../../build/source/projects/websocketclient/generic_max.pb.h"

class protobuf_decoder {
public:

	protobuf_decoder() {

	}

	void decode(proto_message_wrapper& wrap) {

		if (wrap.proto()) {
			delete wrap.proto();
		}

		wrap.proto() = new generic_max();

		static_cast<generic_max*>(wrap.proto())->ParseFromArray(wrap.data(), wrap.size());
	}

	void encode(proto_message_wrapper& wrap) {
		wrap.vect().reserve(static_cast<generic_max*>(wrap.proto())->ByteSize());
		static_cast<generic_max*>(wrap.proto())->SerializeToArray(wrap.vect().data(), wrap.vect().size());
	}

private:

	std::array<std::string, 2> known_messages {
		"iiwa_state",
		"generic_max"
	}; 
};