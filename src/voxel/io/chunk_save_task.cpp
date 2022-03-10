#include "stdafx.h"

#include "io/iomanager.h"

#include "voxel/io/chunk_save_task.h"

static const char* REMOVE_ME = "REMOVE_ME_ASAAAAP";

void hvox::ChunkSaveTask::execute(ChunkFileTaskThreadState*, ChunkFileTaskTaskQueue*) {
    if (!chunk_data_file.is_open()) {
        m_iomanager->memory_map_file(hio::fs::path(REMOVE_ME), chunk_data_file);
    }

    
}
