# Networking Extensions for Max/MSP - [![Build Status](https://travis-ci.com/jonasohland/Max-Net.svg?branch=master)](https://travis-ci.com/jonasohland/Max-Net)

## Building 
To successfully build  these externals you will need:
- Boost.Asio
- Boost.Beast
- Boost.System
- Boost.Date_Time
- Google Protocol Buffers

You can get the newest Boost libraries at [boost](http://www.boost.org). The protobuf library can be found [here](https://github.com/protocolbuffers/protobuf). Please note that on Windows the protobuf library needs to be compiled with `-Dprotobuf_MSVC_STATIC_RUNTIME=OFF` to link against the MSVC runtime library dynamically. Read more on this [here](https://stackoverflow.com/questions/35116437/errors-when-linking-to-protobuf-3-on-ms-visual-c). 

## Goals
This Project aims to provide a simple and modern c++ api for implementing io applications using boost asio in the form of max-msp external objects. These objects may implement things such as:

- Realtime Audio Streaming
- Cloud based speech to text or text to speech
- Logging
- General purpose http / websocket endpoints
- Social Media interaction
- (maybe some jitter stuff idk)
