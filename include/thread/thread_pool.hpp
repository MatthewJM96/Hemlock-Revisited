#ifndef __hemlock_threading_thread_pool_hpp
#define __hemlock_threading_thread_pool_hpp

namespace hemlock {
    namespace thread {
        template <typename ThreadState>
        concept InterruptibleState = requires(ThreadState state)
        {
            std::is_same_v<decltype(state.stop),    bool>;
            std::is_same_v<decltype(state.suspend), bool>;
        };

        struct BasicThreadContext {
            volatile bool stop;
            volatile bool suspend;
        };

        template <InterruptibleState ThreadState>
        class IThreadTask;

        template <InterruptibleState ThreadState>
        struct HeldTask {
            IThreadTask<ThreadState>* task;
            bool should_delete;
        };

        template <InterruptibleState ThreadState>
        using TaskQueue = moodycamel::BlockingConcurrentQueue<HeldTask<ThreadState>>;

        template <InterruptibleState ThreadState>
        struct Thread {
            std::thread thread;
            struct State {
                ThreadState context = {};
                moodycamel::ConsumerToken consumer_token;
                moodycamel::ProducerToken producer_token;
            } state;
        };
        template <InterruptibleState ThreadState>
        using Threads = std::vector<Thread<ThreadState>>;

        template <InterruptibleState ThreadState>
        class IThreadTask {
        public:
            IThreadTask() { /* Empty. */ }
            virtual ~IThreadTask() { /* Empty. */ }

            /**
             * @brief If any special handling is needed to clean
             * up task, override this.
             */
            virtual void dispose() { /* Empty */ }

            /**
             * @brief Executes the task, this must be implemented
             * by inheriting tasks.
             *
             * @param state The thread state, including tokens for
             * interacting with task queue, and thread pool specific
             * context.
             * @param task_queue The task queue, can be interacted with
             * for example if a task needs to chain a follow-up task.
             */
            virtual void execute(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue) = 0;

            /**
             * @brief Tracks completion state of the task.
             */
            volatile bool is_finished = false;
        };

        template <InterruptibleState ThreadState>
        class ThreadPool;

        template <InterruptibleState ThreadState>
        using ThreadMainFunc = Delegate<void(typename Thread<ThreadState>::State*, TaskQueue<ThreadState>*)>;

        /**
         * @brief A basic main function of threads.
         * 
         * @param state The thread state, including tokens for
         * interacting with task queue, and thread pool specific
         * context.
         * @param task_queue The task queue, can be interacted with
         * for example if a task needs to chain a follow-up task.
         */
        template <InterruptibleState ThreadState>
        void basic_thread_main(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue);

        template <InterruptibleState ThreadState>
        class ThreadPool {
        public:
            ThreadPool() :
                m_is_initialised(false),
                m_producer_token(moodycamel::ProducerToken(m_tasks))
            { /* Empty. */ }
            ~ThreadPool() { /* Empty. */ }

            /**
             * @brief Initialises the thread pool with the specified
             * number of threads.
             *
             * @param thread_count The number of threads the pool shall
             * possess.
             */
            void init(ui32 thread_count, ThreadMainFunc<ThreadState> thread_main_func = ThreadMainFunc<ThreadState>{basic_thread_main<ThreadState>});
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

            ThreadMainFunc<ThreadState> m_thread_main_func;
            Threads<ThreadState>        m_threads;
            TaskQueue<ThreadState>      m_tasks;
            moodycamel::ProducerToken   m_producer_token;
        };
    }
}
namespace hthread = hemlock::thread;

#include "thread/thread_pool.inl"

#endif // __hemlock_threading_thread_pool_hpp
