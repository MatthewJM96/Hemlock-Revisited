#ifndef __hemlock_thread_thread_pool_hpp
#define __hemlock_thread_thread_pool_hpp

#include "thread/task.hpp"
#include "thread/task_queue.hpp"
#include "thread/thread_state.hpp"

namespace hemlock {
    namespace thread {
        enum class ThreadpoolTimingResolution {
            NONE,
            ON_SUSPEND,
            ON_TASK_COMPLETION
        };

        template <IsThreadState ThreadState>
        struct Thread {
            std::thread thread;

            ThreadState state;
        };

        template <IsThreadState ThreadState>
        using Threads = std::vector<Thread<ThreadState>>;

        template <
            IsThreadState              ThreadState,
            IsTaskQueue                TaskQueue = BasicTaskQueue,
            ThreadpoolTimingResolution Timing    = ThreadpoolTimingResolution::NONE>
        class ThreadPool;

        template <
            IsThreadState              ThreadState,
            IsTaskQueue                TaskQueue,
            ThreadpoolTimingResolution Timing>
        using ThreadMainFunc = Delegate<void(ThreadState*, TaskQueue*)>;
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#include "thread/basic_thread_main.hpp"

namespace hemlock {
    namespace thread {
        template <
            IsThreadState              ThreadState,
            IsTaskQueue                TaskQueue,
            ThreadpoolTimingResolution Timing>
        class ThreadPool {
        public:
            using _ThreadMainFunc = ThreadMainFunc<ThreadState, TaskQueue, Timing>;

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
                ui32                                           thread_count,
                ThreadMainFunc<ThreadState, TaskQueue, Timing> thread_main_func
                = ThreadMainFunc<
                    ThreadState,
                    TaskQueue,
                    Timing>{ basic_thread_main<ThreadState, TaskQueue, Timing> }
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
            void add_task(QueuedTask task);
            /**
             * @brief Adds a set of tasks to the task queue.
             *
             * NOTE: This should only ever be called
             * from thread owning the thread pool.
             *
             * @param task The tasks to add.
             */
            void add_tasks(QueuedTask tasks[], size_t task_count);

            /**
             * @brief Adds a task to the task queue.
             *
             * NOTE: This can be called from any thread,
             * but comes with a performance penalty as
             * no producer token is used.
             *
             * @param task The task to add.
             */
            void threadsafe_add_task(QueuedTask task);
            /**
             * @brief Adds a set of tasks to the task queue.
             *
             * NOTE: This can be called from any thread,
             * but comes with a performance penalty as
             * no producer token is used.
             *
             * @param task The tasks to add.
             */
            void threadsafe_add_tasks(QueuedTask tasks[], size_t task_count);

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

            _ThreadMainFunc           m_thread_main_func;
            Threads<ThreadState>      m_threads;
            TaskQueue                 m_tasks;
            moodycamel::ProducerToken m_producer_token;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#include "thread/thread_pool.inl"

#endif  // __hemlock_thread_thread_pool_hpp
