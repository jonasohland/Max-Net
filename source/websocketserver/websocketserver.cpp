#include <mutex>
#include <thread>

#include "../shared/net_url.h"
#include "../shared/connection.h"

#include "../shared/messages/generic_max_message.h"
#include "../shared/messages/proto_message_wrapper.h"
#include "../shared/messages/proto_message_base.h"


#include "../shared/devices/devices.h"

#include "../shared/ohlano_min.h"

#include "c74_min.h"


class websocketserver : public c74::min::object<websocketserver> {

	c74::min::attribute<long long> num_connections{ this, "max connections", 0, min_wrap_member(&websocketserver::handle_num_connections_change) };

public:

	explicit websocketserver(const c74::min::atoms& args = {}) {
		if (args.size() > 0) {

		}
	}

	/* ------------- handlers ------------- */

	c74::min::atoms handle_num_connections_change(const c74::min::atoms& args, int inlet) {
		return args;
	}

};


void ext_main(void* r) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	c74::min::wrap_as_max_external<websocketserver>("websocketserver", __FILE__, r);
}