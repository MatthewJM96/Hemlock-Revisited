#ifndef __hemlock_thread_thread_workflow_hpp
#define __hemlock_thread_thread_workflow_hpp

#include "thread/thread_workflow_state.hpp"

namespace hemlock {
    namespace thread {
        template <InterruptibleState ThreadState>
        class IThreadWorkflowTask : public IThreadTask<ThreadState> {
        public:
            IThreadWorkflowTask() { /* Empty. */ }
            virtual ~IThreadWorkflowTask() { /* Empty. */ }
            /**
             * @brief If any special handling is needed to clean
             * up task, override this.
             */
            virtual void dispose() override;

            /**
             * @brief Set up necessary state for task to schedule
             * subsequent tasks in workflow.
             *
             * @param tasks The task list for the workflow instance.
             * @param task_idx The index of this task that is to execute.
             * @param dag The DAG of the workflow.
             * @param task_completion_states The state of completion of
             * tasks feeding into each task.
             */
            void set_workflow_metadata(
                 ThreadWorkflowTasksView<ThreadState> tasks,
                                 ThreadWorkflowTaskID task_idx,
                                   ThreadWorkflowDAG* dag,
                     ThreadWorkflowTaskCompletionView task_completion_states
            );

            /**
             * @brief Handles firing off the run_task function and
             * putting the next task on the queue on completion.
             *
             * @param state The thread state, including tokens for
             * interacting with task queue, and thread pool specific
             * context.
             * @param task_queue The task queue, can be interacted with
             * for example if a task needs to chain a follow-up task.
             */
            void execute(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue) final;

            /**
             * @brief Executes the task, this must be implemented
             * by inheriting tasks.
             * 
             * @param state The thread state, including tokens for
             * interacting with task queue, and thread pool specific
             * context.
             * @param task_queue The task queue, can be interacted with
             * for example if a task needs to chain a follow-up task.
             * @return True if the next tasks in the workflow should fire,
             * false otherwise.
             */
            virtual bool run_task(typename Thread<ThreadState>::State* state, TaskQueue<ThreadState>* task_queue) = 0;
        protected:
            ThreadWorkflowTasksView<ThreadState>    m_tasks;
            ThreadWorkflowTaskID                    m_task_idx;
            ThreadWorkflowDAG*                      m_dag;
            ThreadWorkflowTaskCompletionView        m_task_completion_states;
        };

        template <InterruptibleState ThreadState>
        class ThreadWorkflow {
        public:
            ThreadWorkflow()  { /* Empty. */ }
            ~ThreadWorkflow() { /* Empty. */ }

            void init(ThreadWorkflowDAG* dag, ThreadPool<ThreadState>* thread_pool);
            void dispose();

            void run(ThreadWorkflowTasksView<ThreadState> tasks);
        protected:
            ThreadWorkflowDAG*          m_dag;
            ThreadPool<ThreadState>*    m_thread_pool;
        };
    }
}
namespace hthread = hemlock::thread;

#include "thread/thread_workflow.inl"

#endif // __hemlock_thread_thread_workflow_hpp
