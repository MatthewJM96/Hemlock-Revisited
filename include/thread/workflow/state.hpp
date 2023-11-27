#ifndef __hemlock_thread_workflow_state_hpp
#define __hemlock_thread_workflow_state_hpp

#include "thread/thread_pool.hpp"

namespace hemlock {
    namespace thread {
        class IThreadWorkflowTask;

        struct HeldWorkflowTask {
            IThreadWorkflowTask* task;
            bool                              delete_on_complete;
        };

        using ThreadWorkflowTaskID = ui32;

        struct ThreadWorkflowTasksView {
            hmem::Handle<HeldWorkflowTask[]> tasks;
            ui32                                          count;
        };

        using ThreadWorkflowTaskCompletion = std::atomic<ui32>;

        struct ThreadWorkflowTaskCompletionView {
            hmem::Handle<ThreadWorkflowTaskCompletion[]> completion_states;
            ui32                                         count;
        };

        using ThreadWorkflowTaskIntoCount = std::vector<ThreadWorkflowTaskID>;
        using ThreadWorkflowTaskIndexList = std::unordered_set<ThreadWorkflowTaskID>;
        using ThreadWorkflowTaskGraph
            = std::unordered_multimap<ThreadWorkflowTaskID, ThreadWorkflowTaskID>;

        struct ThreadWorkflowDAG {
            ui32                        task_count = 0;
            ThreadWorkflowTaskIntoCount into_counts;
            ThreadWorkflowTaskIndexList entry_tasks;
            ThreadWorkflowTaskGraph     graph;
        };
    }  // namespace thread
}  // namespace hemlock
namespace hthread = hemlock::thread;

#endif  // __hemlock_thread_workflow_state_hpp
