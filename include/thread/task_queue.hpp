#ifndef __hemlock_thread_task_queue_hpp
#define __hemlock_thread_task_queue_hpp

#include "basic_concepts.hpp"

namespace hemlock {
    namespace thread {
        // TODO(Matthew): this constrains the threadpool to still be aware of producer
        //                and consumer tokens. We should come up with an API that allows
        //                a moodycamel::ConcurrentQueue to be used under the hood but
        //                with token choice being handled in the abstraction layer
        //                around it. We also need this API to support blocking for some
        //                time - a straightforward 1:1 with moodycamel::
        //                BlockingConcurrentQueue no doubt works for this.
        //                  remember that the API needs to handle a few different queue
        //                  algorithms, including a prioritised queue (EEVDF?), a
        //                  checkerboard queue, and maybe a pipelined queue

        template <typename Candidate, typename ItemType>
        concept IsTaskQueue = requires (
            Candidate                 t,
            ItemType                  i,
            moodycamel::ProducerToken p,
            moodycamel::ConsumerToken c,
            size_t                    n
        ) {
                                  {
                                      t.enqueue(i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.enqueue(std::move(i))
                                      } -> std::same_as<bool>;
                                  {
                                      t.enqueue(p, i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.enqueue(p, std::move(i))
                                      } -> std::same_as<bool>;
                                  {
                                      t.enqueue_bulk(&i, n)
                                      } -> std::same_as<bool>;
                                  {
                                      t.enqueue_bulk(p, &i, n)
                                      } -> std::same_as<bool>;

                                  {
                                      t.try_enqueue(i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_enqueue(std::move(i))
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_enqueue(p, i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_enqueue(p, std::move(i))
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_enqueue_bulk(&i, n)
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_enqueue_bulk(p, &i, n)
                                      } -> std::same_as<bool>;

                                  {
                                      t.try_dequeue(i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_dequeue(c, i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_dequeue_bulk(&i, n)
                                      } -> std::same_as<size_t>;
                                  {
                                      t.try_dequeue_bulk(c, &i, n)
                                      } -> std::same_as<size_t>;

                                  {
                                      t.try_dequeue_from_producer(p, i)
                                      } -> std::same_as<bool>;
                                  {
                                      t.try_dequeue_bulk_from_producer(p, &i, n)
                                      } -> std::same_as<size_t>;

                                  {
                                      t.size_approx()
                                      } -> std::same_as<size_t>;
                              };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_task_queue_hpp
