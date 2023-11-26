
template <typename DataType>
void hvox::set_per_block_data(
    DataType*                buffer,
    hvox::BlockChunkPosition start,
    hvox::BlockChunkPosition end,
    DataType                 data
) {
    /*
     * If we span the whole chunk, we just fill the whole buffer.
     */
    if (start == hvox::BlockChunkPosition{ 0 }
        && end == hvox::BlockChunkPosition{ CHUNK_LENGTH - 1 })
    {
        std::fill_n(buffer, CHUNK_VOLUME, data);
        /*
         * If we span the XY plane, then we set the whole cuboid as all
         * elements it touches are contiguous in memory.
         */
    } else if (start.xy() == hvox::BlockChunkPosition2D{ 0 } && end.xy() == hvox::BlockChunkPosition2D{ CHUNK_LENGTH - 1 })
    {
        auto batch_size = CHUNK_AREA * (1 + end.z - start.z);
        auto start_idx  = block_index({ 0, 0, start.z });
        std::fill_n(&(buffer[start_idx]), batch_size, data);
        /*
         * If we span the X line, then we set squares in XY plane one at a time.
         */
    } else if (start.x == 0 && end.x == CHUNK_LENGTH - 1) {
        auto batch_size = CHUNK_LENGTH * (1 + end.y - start.y);
        for (auto z = start.z; z <= end.z; ++z) {
            auto start_idx = block_index({ 0, start.y, z });
            std::fill_n(&(buffer[start_idx]), batch_size, data);
        }
        /*
         * This is a worst-case bulk setting, we have to set each X segment
         * one at a time.
         */
    } else {
        auto batch_size = 1 + end.x - start.x;
        for (auto z = start.z; z <= end.z; ++z) {
            for (auto y = start.y; y <= end.y; ++y) {
                auto start_idx = block_index({ start.x, y, z });
                std::fill_n(&(buffer[start_idx]), batch_size, data);
            }
        }
    }
}

template <typename DataType>
void hvox::set_per_block_data(
    DataType*                buffer,
    hvox::BlockChunkPosition start,
    hvox::BlockChunkPosition end,
    DataType*                data
) {
    /*
     * If we span the whole chunk, we just copy in the whole buffer.
     */
    if (start == BlockChunkPosition{ 0 } && end == BlockChunkPosition{ CHUNK_LENGTH }) {
        std::memcpy(buffer, data, CHUNK_VOLUME);
        /*
         * If we span the XY plane, then we copy the whole cuboid as all
         * elements it touches are contiguous in memory.
         */
    } else if (start.xy() == BlockChunkPosition2D{ 0 } && end.xy() == BlockChunkPosition2D{ CHUNK_LENGTH })
    {
        auto batch_size = CHUNK_AREA * (end.z - start.z);
        auto start_idx  = block_index({ 0, 0, start.z });
        std::memcpy(&(buffer[start_idx]), data, batch_size);
        /*
         * If we span the X line, then we copy squares in XY plane one at a time.
         */
    } else if (start.x == 0 && end.x == CHUNK_LENGTH) {
        auto batch_size = CHUNK_LENGTH * (end.y - start.y);
        for (auto z = start.z; z < end.z; ++z) {
            auto chunk_blocks_start_idx = block_index({ 0, start.y, z });
            auto new_blocks_start_idx   = batch_size * (z - start.z);
            std::memcpy(
                &(buffer[chunk_blocks_start_idx]),
                &(data[new_blocks_start_idx]),
                batch_size
            );
        }
        /*
         * This is a worst-case bulk setting, we have to copy each X segment
         * one at a time.
         */
    } else {
        auto batch_size = end.x - start.x;
        for (auto z = start.z; z < end.z; ++z) {
            for (auto y = start.y; y < end.y; ++y) {
                auto chunk_blocks_start_idx = block_index({ start.x, y, z });
                auto new_blocks_start_idx
                    = batch_size * (y - start.y)
                      + batch_size * (end.y - start.y) * (z - start.z);
                std::memcpy(
                    &(buffer[chunk_blocks_start_idx]),
                    &(data[new_blocks_start_idx]),
                    batch_size
                );
            }
        }
    }
}
