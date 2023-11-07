#ifndef __hemlock_thread_thread_pool_hpp
#define __hemlock_thread_thread_pool_hpp

namespace hemlock {
    namespace thread {
        enum class ThreadpoolTimingResolution {
            NONE,
            ON_SUSPEND,
            ON_TASK_COMPLETION
        };

        template <typename Candidate>
        concept IsThreadState = requires (Candidate state) {
                                    std::is_same_v<
                                        decltype(state.consumer_token),
                                        moodycamel::ConsumerToken>;
                                    std::is_same_v<
                                        decltype(state.producer_token),
                                        moodycamel::ProducerToken>;
                                    std::is_same_v<decltype(state.stop), bool>;
                                    std::is_same_v<decltype(state.suspend), bool>;
                                };

        struct BasicThreadState {
            moodycamel::ConsumerToken consumer_token;
            moodycamel::ProducerToken producer_token;

            volatile bool stop;
            volatile bool suspend;
        };

        template <IsThreadState ThreadState>
        class IThreadTask;

        template <IsThreadState ThreadState>
        struct HeldTask {
            IThreadTask<ThreadState>* task;
            bool                      should_delete;
        };

        template <IsThreadState ThreadState>
        using TaskQueue = moodycamel::BlockingConcurrentQueue<HeldTask<ThreadState>>;

        template <IsThreadState ThreadState>
        struct Thread {
            std::thread thread;

            ThreadState state;
        };

        template <IsThreadState ThreadState>
        using Threads = std::vector<Thread<ThreadState>>;

        template <IsThreadState ThreadState>
        class IThreadTask {
        public:
            IThreadTask() { /* Empty. */
            }

            virtual ~IThreadTask() { /* Empty. */
            }

            /**
             * @brief If any special handling is needed to clean
             * up task, override this.
             */
            virtual void dispose() { /* Empty */
            }

            /**
             * @brief Executes the task, this must be implemented
             * by inheriting tasks.
             *
             * @param state The thread state, including tokens for
             * interacting with task queue, and thread pool specific
             * context.
             * @param task_queue The task queue, can be interacted with
             * for example if a task needs to chain a follow-up task.
             * @return True if the task completed, false if it needs to
             * be re-queued.
             */
            virtual bool execute(ThreadState* state, TaskQueue<ThreadState>* task_queue)
                = 0;
        };

        template <
            IsThreadState              ThreadState,
            ThreadpoolTimingResolution Timing = ThreadpoolTimingResolution::NONE>
        class ThreadPool;

        template <IsThreadState ThreadState, ThreadpoolTimingResolution Timing>
        using ThreadMainFunc = Delegate<void(ThreadState*, TaskQueue<ThreadState>*)>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#include "thread/basic_thread_main.hpp"

namespace hemlock {
    namespace thread {
        template <IsThreadState ThreadState, ThreadpoolTimingResolution Timing>
        class ThreadPool {
        public:
            ThreadPool() :
                m_is_initialised(false),
                m_producer_token(moodycamel::ProducerToken(m_tasks)) { /* Empty. */
            }

            ~ThreadPool() { dispose(); }

            /**
             * @brief Initialises the thread pool with the specified
             * number of threads.
             *
             * @param thread_count The number of threads the pool shall
             * possess.
             */
            void init(
                ui32                                thread_count,
                ThreadMainFunc<ThreadState, Timing> thread_main_func = ThreadMainFunc<
                    ThreadState,
                    Timing>{ basic_thread_main<ThreadState, Timing> }
            );
            /**
             * @brief Cleans up the thread pool, bringing all threads
             * to a stop.
             */
            void dispose();

            /**
             * @brief Suspends activity in the thread pool. Ongoing
             * tasks are allowed to complete, but any remaining in the
             * queue or added while the thread pool is suspended will
             * not be processed until the thread pool is resumed.
             */
            void suspend();
            /**
             * @brief Resumes activity in the thread pool. The queue
             * will once more be processed.
             */
            void resume();

            /**
             * @brief Adds a task to the task queue.
             *
             * NOTE: This should only ever be called
             * from thread owning the thread pool.
             *
             * @param task The task to add.
             */
            void add_task(HeldTask<ThreadState> task);
            /**
             * @brief Adds a set of tasks to the task queue.
             *
             * NOTE: This should only ever be called
             * from thread owning the thread pool.
             *
             * @param task The tasks to add.
             */
            void add_tasks(HeldTask<ThreadState> tasks[], size_t task_count);

            /**
             * @brief Adds a task to the task queue.
             *
             * NOTE: This can be called from any thread,
             * but comes with a performance penalty as
             * no producer token is used.
             *
             * @param task The task to add.
             */
            void threadsafe_add_task(HeldTask<ThreadState> task);
            /**
             * @brief Adds a set of tasks to the task queue.
             *
             * NOTE: This can be called from any thread,
             * but comes with a performance penalty as
             * no producer token is used.
             *
             * @param task The tasks to add.
             */
            void threadsafe_add_tasks(HeldTask<ThreadState> tasks[], size_t task_count);

            /**
             * @brief The number of threads held by the thread pool.
             */
            void num_threads() { return m_threads.size(); }

            /**
             * @brief The approximate number of tasks held by the thread pool.
             */
            void approx_num_tasks() { return m_tasks.size_approx(); }
        protected:
            bool m_is_initialised;

            ThreadMainFunc<ThreadState, Timing> m_thread_main_func;
            Threads<ThreadState>                m_threads;
            TaskQueue<ThreadState>              m_tasks;
            moodycamel::ProducerToken           m_producer_token;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#include "thread/thread_pool.inl"

#endif  // __hemlock_thread_thread_pool_hpp
