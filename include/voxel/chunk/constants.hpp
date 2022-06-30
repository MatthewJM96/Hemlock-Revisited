#ifndef __hemlock_voxel_chunk_constants_hpp
#define __hemlock_voxel_chunk_constants_hpp

#ifndef CHUNK_LENGTH
#define CHUNK_LENGTH 32
#endif

#undef CHUNK_AREA
#define CHUNK_AREA CHUNK_LENGTH * CHUNK_LENGTH

#undef CHUNK_VOLUME
#define CHUNK_VOLUME CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH

#endif // __hemlock_voxel_chunk_constants_hpp
