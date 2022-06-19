
template <typename DataType>
void hvox::set_per_block_data( DataType* buffer,
                hvox::BlockChunkPosition start_block_position,
                hvox::BlockChunkPosition end_block_position,
                                DataType data )
{
    /*
     * If we span the whole chunk, we just fill the whole buffer.
     */
    if (start_block_position == hvox::BlockChunkPosition{0} && end_block_position == hvox::BlockChunkPosition{CHUNK_LENGTH - 1}) {
        std::fill_n(buffer, CHUNK_VOLUME, data);
    /*
     * If we span the XY plane, then we set the whole cuboid as all
     * elements it touches are contiguous in memory.
     */
    } else if (start_block_position.xy() == hvox::BlockChunkPosition2D{0} && end_block_position.xy() == hvox::BlockChunkPosition2D{CHUNK_LENGTH - 1}) {
        auto batch_size = CHUNK_AREA * (1 + end_block_position.z - start_block_position.z);
        auto start_idx = block_index({
            0, 0, start_block_position.z
        });
        std::fill_n(&(buffer[start_idx]), batch_size, data);
    /*
     * If we span the X line, then we set squares in XY plane one at a time.
     */
    } else if (start_block_position.x == 0 && end_block_position.x == CHUNK_LENGTH - 1) {
        auto batch_size = CHUNK_LENGTH * (1 + end_block_position.y - start_block_position.y);
        for (auto z = start_block_position.z; z <= end_block_position.z; ++z) {
            auto start_idx = block_index({
                0, start_block_position.y, z
            });
            std::fill_n(&(buffer[start_idx]), batch_size, data);
        }
    /*
     * This is a worst-case bulk setting, we have to set each X segment
     * one at a time.
     */
    } else {
        auto batch_size = 1 + end_block_position.x - start_block_position.x;
        for (auto z = start_block_position.z; z <= end_block_position.z; ++z) {
            for (auto y = start_block_position.y; y <= end_block_position.y; ++y) {
                auto start_idx = block_index({
                    start_block_position.x, y, z
                });
                std::fill_n(&(buffer[start_idx]), batch_size, data);
            }
        }
    }
}

template <typename DataType>
void hvox::set_per_block_data( DataType* buffer,
                hvox::BlockChunkPosition start_block_position,
                hvox::BlockChunkPosition end_block_position,
                              DataType* data )
{
    /*
     * If we span the whole chunk, we just copy in the whole buffer.
     */
    if (start_block_position == BlockChunkPosition{0} && end_block_position == BlockChunkPosition{CHUNK_LENGTH}) {
        std::memcpy(buffer, data, CHUNK_VOLUME);
    /*
     * If we span the XY plane, then we copy the whole cuboid as all
     * elements it touches are contiguous in memory.
     */
    } else if (start_block_position.xy() == BlockChunkPosition2D{0} && end_block_position.xy() == BlockChunkPosition2D{CHUNK_LENGTH}) {
        auto batch_size = CHUNK_AREA * (end_block_position.z - start_block_position.z);
        auto start_idx = block_index({
            0, 0, start_block_position.z
        });
        std::memcpy(&(buffer[start_idx]), data, batch_size);
    /*
     * If we span the X line, then we copy squares in XY plane one at a time.
     */
    } else if (start_block_position.x == 0 && end_block_position.x == CHUNK_LENGTH) {
        auto batch_size = CHUNK_LENGTH * (end_block_position.y - start_block_position.y);
        for (auto z = start_block_position.z; z < end_block_position.z; ++z) {
            auto chunk_blocks_start_idx = block_index({
                0, start_block_position.y, z
            });
            auto new_blocks_start_idx = batch_size * (z - start_block_position.z);
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
        auto batch_size = end_block_position.x - start_block_position.x;
        for (auto z = start_block_position.z; z < end_block_position.z; ++z) {
            for (auto y = start_block_position.y; y < end_block_position.y; ++y) {
                auto chunk_blocks_start_idx = block_index({
                    start_block_position.x, y, z
                });
                auto new_blocks_start_idx =
                        batch_size * (y - start_block_position.y)
                            + batch_size * (end_block_position.y - start_block_position.y) * (z - start_block_position.z);
                std::memcpy(
                    &(buffer[chunk_blocks_start_idx]),
                    &(data[new_blocks_start_idx]),
                    batch_size
                );
            }
        }
    }
}