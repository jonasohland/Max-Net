#pragma once

#include "c74_min.h"
#include "../ohlano.h"
#include "../messages/proto_message_base.h"
#include "../../../build/source/websocketclient/generic_max.pb.h"

namespace ohlano {
    
	class max_message : public proto_message_base<generic_max> {

	public:
		class max_message_allocator {
		public:

			max_message* allocate() {
				auto msg = new max_message();
				alloc_msg_count++;
				return msg;
			}

			void deallocate(const max_message* msg) {
				alloc_msg_count--;
				delete msg;
			}

			void deallocate(max_message* msg) {
				alloc_msg_count--;
				delete msg;
			}

			~max_message_allocator() {
				assert(alloc_msg_count.load() == 0);
			}

			max_message_allocator() {
				alloc_msg_count.store(0);
			}

		private:
			std::set<max_message*> messages;
			std::atomic<size_t> alloc_msg_count;

		};

	public:

		typedef max_message_allocator factory;

		void push_atom(c74::min::atom atom_in) {

			auto new_atom = proto()->add_atom();

			switch (atom_in.a_type) {
			case c74::max::e_max_atomtypes::A_LONG :
				new_atom->set_type(A_LONG);
				new_atom->set_int_(atom_in);
				break;
			case c74::max::e_max_atomtypes::A_FLOAT:
				new_atom->set_type(A_FLOAT);
				new_atom->set_float_(atom_in);
				break;
			case c74::max::e_max_atomtypes::A_SYM:
				new_atom->set_type(A_SYMBOL);
				new_atom->set_string_(atom_in);
				break;
			default:
				DBG("unknown atom!");
			}
			
			
		}

		void push_atoms(const c74::min::atoms& atms) {
			for (const auto& atm : atms) {
				push_atom(atm);
			}
		}

		void push_atomarray(c74::min::atoms::const_iterator it_begin, c74::min::atoms::const_iterator it_end, c74::max::e_max_atomtypes type) {

			auto new_atm = proto()->add_atom();

			if (type == c74::max::e_max_atomtypes::A_LONG) {

				auto arr = new atom_int_array();

				arr->mutable_values()->Reserve((int) (it_end - it_begin));

				std::copy(it_begin, it_end, google::protobuf::internal::RepeatedFieldBackInsertIterator<google::protobuf::int64>(arr->mutable_values()));

				new_atm->set_allocated_int_array_(arr);
				new_atm->set_type(A_ARR_LONG);
			}
			else if (type == c74::max::e_max_atomtypes::A_FLOAT) {

				auto arr = new atom_float_array();

				arr->mutable_values()->Reserve((int) (it_end - it_begin));

				std::copy(it_begin, it_end, google::protobuf::internal::RepeatedFieldBackInsertIterator<float>(arr->mutable_values()));

				new_atm->set_allocated_float_array_(arr);
				new_atm->set_type(A_ARR_FLOAT);
			}
			else {
				assert(false);
			}
		}

		c74::min::atoms get_atoms() const {

			c74::min::atoms out_atoms;

			DBG("unpacking: ", proto()->DebugString());
			
			for (const auto& atom: proto()->atom()) {
				switch (atom.type()) {
				case A_LONG:
					out_atoms.emplace_back(static_cast<c74::max::t_atom_long>(atom.int_()));
					break;
				case A_FLOAT:
					out_atoms.emplace_back(static_cast<c74::max::t_atom_float>(atom.float_()));
					break;
				case A_SYMBOL:
					out_atoms.emplace_back(atom.string_());
					break;
				case A_ARR_LONG:
					for (auto const& val : atom.int_array_().values()) {
						out_atoms.emplace_back(val);
					}
					break;
				case A_ARR_FLOAT:
					for (auto const& val : atom.float_array_().values()) {
						out_atoms.emplace_back(val);
					}
					break;
				default:
					DBG("unknown atom!");
				}

			}

			return out_atoms;
		}
		
	};
}
