#ifndef __hemlock_thread_workflow_hpp
#define __hemlock_thread_workflow_hpp

#include "thread/workflow/state.hpp"

namespace hemlock {
    namespace thread {
        class IThreadWorkflowTask : public IThreadTask {
        public:
            IThreadWorkflowTask() : m_task_idx(0), m_dag(nullptr) { /* Empty. */
            }

            virtual ~IThreadWorkflowTask();

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
                ThreadWorkflowTasksView          tasks,
                ThreadWorkflowTaskID             task_idx,
                ThreadWorkflowDAG*               dag,
                ThreadWorkflowTaskCompletionView task_completion_states
            );

            /**
             * @brief Handles firing off the run_task function and
             * putting the next task on the queue on completion.
             *
             */
            virtual bool execute() final;

            /**
             * @brief Executes the task, this must be implemented
             * by inheriting tasks.
             *
             * @return True if the next tasks in the workflow should fire,
             * false otherwise.
             */
            virtual bool run_task() = 0;
        protected:
            ThreadWorkflowTasksView          m_tasks;
            ThreadWorkflowTaskID             m_task_idx;
            ThreadWorkflowDAG*               m_dag;
            ThreadWorkflowTaskCompletionView m_task_completion_states;
        };

        class ThreadWorkflow {
        public:
            ThreadWorkflow() { /* Empty. */
            }

            ~ThreadWorkflow() { /* Empty. */
            }

            void init(ThreadWorkflowDAG* dag, ThreadPool<>* thread_pool);
            void dispose();

            void run(ThreadWorkflowTasksView tasks);
        protected:
            ThreadWorkflowDAG* m_dag;
            ThreadPool<>*      m_thread_pool;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#include "thread/workflow/workflow.inl"

#endif  // __hemlock_thread_workflow_hpp
