#ifndef __hemlock_thread_thread_workflow_builder_h
#define __hemlock_thread_thread_workflow_builder_h

#include "thread/thread_workflow_state.hpp"

namespace hemlock {
    namespace thread {
        class ThreadWorkflowBuilder {
        public:
            ThreadWorkflowBuilder();
            ~ThreadWorkflowBuilder() { /* Empty. */ }

            void init(ThreadWorkflowDAG* dag);
            void dispose();

            ThreadWorkflowDAG* dag() { return m_dag; }

            /**
             * @brief Set the expected number of tasks
             * to be in the DAG.
             * 
             * @param expected_task_count The number of
             * tasks to expect.
             */
            void set_expected_tasks(ui32 expected_task_count);

            /**
             * @brief Marks a new task as existing and
             * returns its ID.
             * 
             * @return The ID of the new task.
             */
            ThreadWorkflowTaskID add_task();
            /**
             * @brief Marks count new tasks as existing
             * and returns the ID of the first of them.
             * 
             * @return The ID of the first new task.
             */
            ThreadWorkflowTaskID add_tasks(ui32 count);

            /**
             * @brief Marks a new task as existing and
             * sets it as depending on the previously
             * added task.
             *
             * @return The ID of the new task.
             */
            ThreadWorkflowTaskID chain_task();
            /**
             * @brief Marks count new tasks as existing
             * and sets them each as depending on the
             * previously added task. This is done
             * sequentially so the second task passed
             * in is set as depending on the first,
             * the third on the second, and so on.
             *
             * @param count The number of tasks to chain.
             *
             * @return The ID of the first-added new task.
             */
            ThreadWorkflowTaskID chain_tasks(ui32 count);
            /**
             * @brief Marks count new tasks as existing
             * and sets them each as depending on the
             * previously added task. This is done
             * in a parallel manner so that all tasks
             * passed in are set to depend on the same
             * task, that being the task added previous
             * to the call to this function.
             *
             * @param count The number of tasks to chain.
             *
             * @return The ID of the first-added new task.
             */
            ThreadWorkflowTaskID chain_tasks_parallel(ui32 count);

            /**
             * @brief Marks a new task as existing and
             * sets it as depending on the specified task.
             *
             * @param from_task The task from which to
             * chain the new task.
             *
             * @return [true, ID] if the task was chained,
             * where ID is that of the newly chained task.
             * [false, 0] otherwise.
             */
            std::tuple<bool, ThreadWorkflowTaskID> chain_task(
                ThreadWorkflowTaskID from_task
            );
            /**
             * @brief Marks count new tasks as existing
             * and sets them each as depending on the
             * previously added task. This is done
             * sequentially so the second task passed
             * in is set as depending on the first,
             * the third on the second, and so on.
             *
             * @param from_task The task from which to
             * begin to chain the new tasks.
             * @param count The number of tasks to chain.
             *
             * @return [true, ID] if the task was chained,
             * where ID is that of the first newly chained
             * task. [false, 0] otherwise.
             */
            std::tuple<bool, ThreadWorkflowTaskID> chain_tasks(
                ThreadWorkflowTaskID from_task,
                                ui32 count
            );
            /**
             * @brief Marks count new tasks as existing
             * and sets them each as depending on the
             * previously added task. This is done
             * in a parallel manner so that all tasks
             * passed in are set to depend on the same
             * task, that being the task added previous
             * to the call to this function.
             *
             * @param from_task The task from which to
             * chain the new tasks.
             * @param count The number of tasks to chain.
             *
             * @return [true, ID] if the task was chained,
             * where ID is that of the first newly chained
             * task. [false, 0] otherwise.
             */
            std::tuple<bool, ThreadWorkflowTaskID> chain_tasks_parallel(
                ThreadWorkflowTaskID from_task,
                                ui32 count
            );

            /**
             * @brief Chains the tasks passed by their IDs.
             *
             * @param first_task The ID of the first task.
             * @param second_task The ID of the task that
             * should follow the first task.
             *
             * @return True if both tasks existed and were
             * chained, false otherwise.
             */
            bool set_task_depends( ThreadWorkflowTaskID first_task,
                                ThreadWorkflowTaskID second_task );
            /**
             * @brief Chains the tasks passed by their IDs.
             *
             * @param task_pairs The ID of the first task
             * and of the task that should follow the
             * first task.
             * @param count The number of task pairs.
             *
             * @return True if both tasks existed and were
             * chained, false otherwise.
             */
            bool set_tasks_depend( std::pair<
                                        ThreadWorkflowTaskID,
                                        ThreadWorkflowTaskID
                                    >* task_pairs,
                                    ui32 count
            );
        protected:
            ThreadWorkflowDAG* m_dag;
        };
    }
}
namespace hthread = hemlock::thread;

#endif // __hemlock_thread_thread_workflow_builder_h
