#include "c74_min.h"
#include "net_url.h"

#define OHLANO_WRAP_DEFERRED_CALL(f)                                           \
  [this](const c74::min::atoms &args, int inlet) -> c74::min::atoms {          \
    this->post([=]() { f(args, inlet); });                                     \
    return args;                                                               \
  }

net_url<> net_url_from_atoms(const c74::min::atoms args) {

    net_url<>::error_code ec;
    net_url<> url;
    net_url<> t_url;

    if (args.size() > 0) {

        for (auto& arg : args) {

            switch (arg.a_type) {
            case c74::max::e_max_atomtypes::A_SYM:

                if (!url) {

                    t_url = net_url<>(arg, ec);

                    if (ec != net_url<>::error_code::SUCCESS)
                        // cerr << "symbol argument could not be decoded to an url" << endl;

                        if (url.has_port() && t_url.has_port()) {
                            // cerr << "Found multiple port arguments!" << endl;
                        }
                    url = t_url;
                }

                break;

            case c74::max::e_max_atomtypes::A_FLOAT:
                // cerr << "float not supported as argument" << endl;
                break;
            case c74::max::e_max_atomtypes::A_LONG:
                // cout << "long arg: " << std::string(arg) << endl;
                if (!url.has_port()) {
                    url.set_port(std::to_string(static_cast<int>(arg)));
                }
                break;
            default:
                //cerr << "unsupported argument type" << endl;
                break;
            }

        }
    }

    return url;
}
