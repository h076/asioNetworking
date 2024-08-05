#ifndef NET_MESSAGE_H_
#define NET_MESSAGE_H_

#include "net_utils.hpp"
#include <type_traits>

namespace olc {

    namespace net {

        // message header is sent at the start of all messages.
        // Template allows us to use 'enum class' to ensure that all messages are valid at compile time
        template <typename T>
        struct message_header {
            T id{};
            uint32_t size = 0; // use unsigned int as size_t could vary on different systems
        };

        // message struct containing message header and message body
        template <typename T>
        struct message {
            message_header<T> header{};
            // vector of unsigned 8 bit integers, so we always work wwith data in bytes
            std::vector<uint8_t> body;

            // return size of entire message packet
            size_t size() const {
                return sizeof(message_header<T>) + body.size();
            }

            // override bit shift left operator tied to std::ostream, for std::cout compatibility
            // allows us to produce description of messages
            friend std::ostream& operator << (std::ostream& os, message<T>& msg) {
                os << "ID : " << int(msg.header.id) << " Size : " << msg.header.size;
                return os;
            }

            // we are going to treat the message body as if it was a stack

            // push any POD-like data into messages
            template <typename DataType>
            friend message<T>& operator << (message<T>& msg, const DataType& data) {
                // handle data types that we cannot trivially serialise
                // such as static variables
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

                // cache current size of body vector
                size_t i = msg.body.size();

                // resize vector by size of data to be pushed
                msg.body.resize(msg.body.size() + sizeof(DataType));

                // copy data into the newly allocated vector space
                std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

                // update message size in message header
                msg.header.size = msg.size();

                // return message object so it can be "chained" together
                return msg;
            }

            // pop data from the body of a message
            template <typename DataType>
            friend message<T>& operator >> (message<T>& msg, DataType& data) {
                // check data is trivial
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pull from vector");

                // cache the location of the start of data to be pulled
                size_t i = msg.body.size() - sizeof(DataType);

                // taking data from the end of the vector means performance is not hindered
                // by uncessary realloc of vector space, caused by taking from the front.

                // copy data from the vector into the users variable
                std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

                // reduce size of vector
                msg.body.resize(i);

                // update message size in message header
                msg.header.size = msg.size();

                // return target message so it can be "chained"
                return msg;
            }
        };
    }
}

#endif // NET_MESSAGE_H_
