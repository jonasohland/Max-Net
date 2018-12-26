#pragma once

#include <../../generic_max.pb.h>

class protobuf_decoder {
public:

	protobuf_decoder() {

	}

	void decode(proto_message_wrapper& wrap) {

		if (wrap.proto()) {
			delete wrap.proto();
		}
		wrap.proto() = new generic_max();
		static_cast<generic_max*>(wrap.proto())->ParseFromArray(wrap.data(), (int) wrap.size());
	}

	void encode(proto_message_wrapper& wrap) {
		wrap.vect().reserve(static_cast<generic_max*>(wrap.proto())->ByteSize());
		static_cast<generic_max*>(wrap.proto())->SerializeToArray(wrap.vect().data(), (int) wrap.vect().size());
	}

private:
};
