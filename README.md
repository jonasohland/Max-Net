# Websockets for Max

## Building
To successfully build  these externals you will need:
- Boost.Asio
- Boost.Beast
- Boost.System
- Boost.Date_Time
- Google Protocol Buffers

You can get the newest Boost libraries at [boost](http://boost.org). The protobuf library can be found [here](https://developers.google.com/protocol-buffers/). Please note that that on Windows the protobuf library needs to be compiled with `Dprotobuf_MSVC_STATIC_RUNTIME=OFF` to link against the MSVC runtime library dynamically. Read more on this [here](https://stackoverflow.com/questions/35116437/errors-when-linking-to-protobuf-3-on-ms-visual-c). 