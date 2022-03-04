#ifndef __hemlock_thread_workflow_hpp
#define __hemlock_thread_workflow_hpp

#include "thread_pool.hpp"

namespace hemlock {
    template <hemlock::InterruptibleState ThreadState>
    class IThreadWorkflowTask;

    template <hemlock::InterruptibleState ThreadState>
    struct HeldWorkflowTask {
        IThreadWorkflowTask<ThreadState>* task;
        bool should_delete;
    };

    using ThreadWorkflowTaskID = i32;

    template <hemlock::InterruptibleState ThreadState>
    using ThreadWorkflowTaskList = std::vector<HeldWorkflowTask<ThreadState>>;

    using ThreadWorkflowTaskIndexList = std::unordered_set<ThreadWorkflowTaskID>;
    using ThreadWorkflowTaskGraph     = std::unordered_multimap<ThreadWorkflowTaskID, ThreadWorkflowTaskID>;

    template <hemlock::InterruptibleState ThreadState>
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
         * @param graph The graph of the workflow.
         */
        void set_workflow_metadata(ThreadWorkflowTaskList<ThreadState>* tasks, ThreadWorkflowTaskID task_idx, ThreadWorkflowTaskGraph* graph);

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
        ThreadWorkflowTaskList<ThreadState>*    m_tasks;
        ThreadWorkflowTaskID                    m_task_idx;
        ThreadWorkflowTaskGraph*                m_graph;
    };

    struct ThreadWorkflowDAG {
        ThreadWorkflowTaskIndexList entry_tasks;
        ThreadWorkflowTaskGraph     graph;
    };

    template <hemlock::InterruptibleState ThreadState>
    class ThreadWorkflow {
    public:
        ThreadWorkflow()  { /* Empty. */ }
        ThreadWorkflow(const ThreadWorkflow& workflow) = delete;
        ThreadWorkflow(ThreadWorkflow&& workflow);
        ~ThreadWorkflow() { /* Empty. */ }

        void init(ThreadWorkflowDAG* dag, ThreadPool<ThreadState>* thread_pool);
        void dispose();

        void start();

        void set_expected_tasks(ui32 expected_task_count);

        /**
         * @brief Adds a task to the workflow without linking
         * it to the workflow DAG. This means that, without a
         * call to chain_task with the ID returned from
         * calling this function, the here-added task will
         * not get executed.
         *
         * @param task The task to add.
         * @return The ID of the task added within this
         * workflow context. 
         */
        ThreadWorkflowTaskID add_task(HeldWorkflowTask<ThreadState> task);
        /**
         * @brief Adds the passed tasks to the workflow without
         * linking them to the workflow DAG. This means that,
         * without a call to chain_task with the IDs returned
         * from calling this function, the here-added tasks
         * will not get executed.
         *
         * @param tasks The tasks to add.
         * @param count The number of tasks to add.
         * @return The ID of the first task added within
         * workflow context. Subsequent tasks added from
         * this call will have successively incremented IDs.
         */
        ThreadWorkflowTaskID add_tasks(HeldWorkflowTask<ThreadState>* tasks, ui32 count);

        /**
         * @brief Simultaneously adds the passed task and
         * chains it to the previously added task.
         *
         * @param task The task to add & chain.
         * @return The ID of the task added within this
         * workflow context.
         */
        ThreadWorkflowTaskID chain_task(HeldWorkflowTask<ThreadState> task);
        /**
         * @brief Adds the passed tasks and chains each to the
         * previously added task. This is done sequentially so
         * the second task passed in is chained to the first,
         * the third to the second, and so on.
         *
         * @param tasks The tasks to add and chain.
         * @param count The number of tasks to add and chain.
         * @return The ID of the first task added within
         * workflow context. Subsequent tasks added from
         * this call will have successively incremented IDs.
         */
        ThreadWorkflowTaskID chain_tasks(HeldWorkflowTask<ThreadState>* tasks, ui32 count);
        /**
         * @brief Adds the passed tasks and chains each to the
         * previously added task. This is done in a parallel
         * manner, so all the tasks passed in here are added
         * to the task added in the previous call. If no
         * previous call was made, then all these tasks
         * become entry points.
         *
         * @param tasks The tasks to add and chain.
         * @param count The number of tasks to add and chain.
         * @return The ID of the first task added within
         * workflow context. Subsequent tasks added from
         * this call will have successively incremented IDs.
         */
        ThreadWorkflowTaskID chain_tasks_parallel(HeldWorkflowTask<ThreadState>* tasks, ui32 count);

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
        bool chain_task(ThreadWorkflowTaskID first_task, ThreadWorkflowTaskID second_task);
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
        bool chain_tasks(std::pair<ThreadWorkflowTaskID, ThreadWorkflowTaskID>* task_pairs, ui32 count);
    protected:
        ThreadWorkflowTaskList<ThreadState> m_tasks;
        ThreadWorkflowDAG*                  m_dag;

        ThreadPool<ThreadState>*            m_thread_pool;
    };
}

#include "thread_workflow.inl"

#endif // __hemlock_thread_workflow_hpp
