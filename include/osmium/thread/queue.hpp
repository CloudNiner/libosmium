#ifndef OSMIUM_THREAD_QUEUE_HPP
#define OSMIUM_THREAD_QUEUE_HPP

/*

This file is part of Osmium (http://osmcode.org/osmium).

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace osmium {

    namespace thread {

        /**
         *  A thread-safe queue.
         */
        template <typename T>
        class Queue {

            mutable std::mutex m_mutex;
            std::queue<T> m_queue;
            std::condition_variable m_data_available;

        public:

            Queue() :
                m_mutex(),
                m_queue(),
                m_data_available() {
            }

            void push(T value) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(std::move(value));
                m_data_available.notify_one();
            }

            size_t push_and_get_size(T&& value) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(std::forward<T>(value));
                m_data_available.notify_one();
                return m_queue.size();
            }

            void push(T value, int) {
                push(value);
            }

            void wait_and_pop(T& value) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_data_available.wait(lock, [this] {
                    return !m_queue.empty();
                });
                value=std::move(m_queue.front());
                m_queue.pop();
            }

            bool try_pop(T& value) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_queue.empty()) {
                    return false;
                }
                value=std::move(m_queue.front());
                m_queue.pop();
                return true;
            }

            bool empty() const {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_queue.empty();
            }

            size_t size() const {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_queue.size();
            }

        }; // class Queue

    } // namespace thread

} // namespace osmium

#endif // OSMIUM_THREAD_QUEUE_HPP
