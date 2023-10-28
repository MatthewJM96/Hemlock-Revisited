#include "stdafx.h"

#include "voxel/cellular_automata/gas_strategy.h"

#include "voxel/chunk/grid.h"

bool hvox::CAGasStrategy::can_run(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>) const {
    return false;
}

void hvox::CAGasStrategy::operator()(hmem::Handle<ChunkGrid>, hmem::Handle<Chunk>)
    const { }
