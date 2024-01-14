#include "stdafx.h"

#include "thread/queue/state.h"

bool hthread::dequeue(
    BasicTaskQueue&      queue,
    OUT QueuedTask*      item,
    TimingRep            timeout,
    OUT BasicTaskQueue** queue_out,
    void*                control_block
) {
    return queue.dequeue(item, timeout, queue_out, control_block);
}
