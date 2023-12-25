#include "stdafx.h"

#include "voxel/state.h"

std::ostream& std::operator<<(std::ostream& os, const hvox::Voxel& voxel) {
    return os << static_cast<ui64>(voxel);
}

std::istream& std::operator<<(std::istream& is, hvox::Voxel& voxel) {
    return is >> *reinterpret_cast<ui64*>(&voxel);
}
