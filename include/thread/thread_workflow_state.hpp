#ifndef __hemlock_thread_thread_workflow_state_hpp
#define __hemlock_thread_thread_workflow_state_hpp

#include "thread_pool.hpp"

namespace hemlock {
    namespace thread {
        template <InterruptibleState ThreadState>
        class IThreadWorkflowTask;

        template <InterruptibleState ThreadState>
        struct HeldWorkflowTask {
            IThreadWorkflowTask<ThreadState>* task;
            bool should_delete;
        };

        using ThreadWorkflowTaskID = i32;

        template <InterruptibleState ThreadState>
        struct ThreadWorkflowTasksView {
            hmem::Handle<HeldWorkflowTask<ThreadState>[]>   tasks;
            ui32                                            count;
        };

        using ThreadWorkflowTaskCompletion = std::atomic<ui32>;
        struct ThreadWorkflowTaskCompletionView {
            hmem::Handle<ThreadWorkflowTaskCompletion[]>    completion_states;
            ui32                                            count;
        };

        using ThreadWorkflowTaskIntoCount = std::vector<ThreadWorkflowTaskID>;
        using ThreadWorkflowTaskIndexList = std::unordered_set<ThreadWorkflowTaskID>;
        using ThreadWorkflowTaskGraph     = std::unordered_multimap<ThreadWorkflowTaskID, ThreadWorkflowTaskID>;

        struct ThreadWorkflowDAG {
            ui32                        task_count = 0;
            ThreadWorkflowTaskIntoCount into_counts;
            ThreadWorkflowTaskIndexList entry_tasks;
            ThreadWorkflowTaskGraph     graph;
        };
    }
}
namespace hthread = hemlock::thread;

#endif // __hemlock_thread_thread_workflow_state_hpp
