#include "stdafx.h"

#include "voxel/cellular_automata/liquid_strategy.h"

#include "voxel/chunk/grid.h"

bool hvox::CALiquidStrategy::can_run(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>)
    const {
    return false;
}

void hvox::CALiquidStrategy::operator()(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>)
    const { }
