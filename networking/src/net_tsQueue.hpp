#ifndef NET_TSQUEUE_H_
#define NET_TSQUEUE_H_
// thread safe network queue
// Must be thread safe as at any one time multiple objects may want to alter the queue
// such as the client interface queue being accesed by the connection object and the client
// object iself

#include "net_utils.hpp"
#include <mutex>

namespace olc {

    namespace net {

        template <typename T>
        class tsqueue {
            public:
                tsqueue() = default;
                // prevent the object to be copied
                tsqueue(const tsqueue<T>&) = delete;
                virtual ~tsqueue() {clear();}

            public:

                // Return and maintain item at the front of the queue
                const T& front() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.front();
                }

                // return and maintain item at the back of the queue
                const T& back() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.back();
                }

                // Add item to back of queue
                void push_back(const T& item) {
                    std::scoped_lock lock(m_oMuxQueue);
                    m_oDeqQueue.emplace_back(std::move(item));
                }

                // Add item to the front of queue
                void push_front(const T& item) {
                    std::scoped_lock lock(m_oMuxQueue);
                    m_oDeqQueue.emplace_front(std::move(item));
                }

                // Is queue empty
                bool empty() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.empty();
                }

                // Return size of queue
                size_t count() {
                    std::scoped_lock lock(m_oMuxQueue);
                    return m_oDeqQueue.size();
                }

                // Clear the queue
                void clear() {
                    std::scoped_lock lock(m_oMuxQueue);
                    m_oDeqQueue.clear();
                }

                // Return front item in queue
                T pop_front() {
                    std::scoped_lock lock(m_oMuxQueue);
                    auto t = std::move(m_oDeqQueue.front());
                    m_oDeqQueue.pop_front();
                    return t;
                }

                // Return back item in queue
                T pop_back() {
                    std::scoped_lock lock(m_oMuxQueue);
                    auto t = std::move(m_oDeqQueue.back());
                    m_oDeqQueue.pop_back();
                    return t;
                }

            protected:

                std::mutex m_oMuxQueue;
                // double ended queue used as our underlying data structure
                std::deque<T> m_oDeqQueue;
        };
    }
}

#endif // NET_TSQUEUE_H_