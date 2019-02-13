#include "proto_message_base.h"
#include "../proto/generated/Movement.pb.h"
#include "../proto/generated/globals.pb.h"

namespace iiwa = de::hsmainz::iiwa::messages::protocolbuffers;

class iiwa_movement_message : public proto_message_base<iiwa::Movement> {

public:

	class allocator {
	public:

		iiwa_movement_message* allocate() {
			return new iiwa_movement_message();
		}

		void deallocate(const iiwa_movement_message* msg) {
			delete msg;
		}
	};

	using factory = allocator;

	void set_move_type(iiwa::Movement::MovementType ty) {
		this->proto()->set_movetype(ty);
	}

	template<typename T>
	void set_joints(T last) {
		this->proto()->jointpositions().add_joints(last);
	}

	template<typename T, typename ...Ts>
	void set_joints(T current, Ts ...rest) {
		this->proto()->jointpositions().add_joints(current);
		set_joints(rest...);
	}
    
    void set_send(bool is_send) const{
        DBG("yay");
    }

private:

	std::vector<double> acc_joints;

};
