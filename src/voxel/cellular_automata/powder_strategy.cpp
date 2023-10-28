#include "stdafx.h"

#include "voxel/cellular_automata/powder_strategy.h"

#include "voxel/chunk/grid.h"

bool hvox::CAPowderStrategy::can_run(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>)
    const {
    return false;
}

void hvox::CAPowderStrategy::operator()(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>)
    const { }
