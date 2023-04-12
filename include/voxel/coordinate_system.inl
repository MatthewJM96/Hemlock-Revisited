template <std::integral IntegralType>
    requires (!std::same_as<hvox::BlockChunkPosition, glm::vec<3, IntegralType, glm::defaultp>>)
hvox::BlockChunkPosition operator+(const hvox::BlockChunkPosition& lhs, const glm::vec<3, IntegralType, glm::defaultp>& rhs) {
    using _vec = glm::vec<3, IntegralType, glm::defaultp>;

    _vec res = static_cast<_vec>(lhs) + rhs;

    return static_cast<hvox::BlockChunkPosition>(res);
}

template <std::integral IntegralType>
    requires (!std::same_as<hvox::BlockChunkPosition, glm::vec<3, IntegralType, glm::defaultp>>)
hvox::BlockChunkPosition operator+(const hvox::BlockChunkPosition& lhs, glm::vec<3, IntegralType, glm::defaultp>&& rhs) {
    using _vec = glm::vec<3, IntegralType, glm::defaultp>;

    _vec res = static_cast<_vec>(lhs) + rhs;

    return static_cast<hvox::BlockChunkPosition>(res);
}

template <std::integral IntegralType>
    requires (!std::same_as<hvox::BlockChunkPosition, glm::vec<3, IntegralType, glm::defaultp>>)
hvox::BlockChunkPosition& operator+=(hvox::BlockChunkPosition& lhs, const glm::vec<3, IntegralType, glm::defaultp>& rhs) {
    using _vec = glm::vec<3, IntegralType, glm::defaultp>;

    _vec res = static_cast<_vec>(lhs) + rhs;

    lhs = static_cast<hvox::BlockChunkPosition>(res);

    return lhs;
}

template <std::integral IntegralType>
    requires (!std::same_as<hvox::BlockChunkPosition, glm::vec<3, IntegralType, glm::defaultp>>)
hvox::BlockChunkPosition& operator+=(hvox::BlockChunkPosition& lhs, glm::vec<3, IntegralType, glm::defaultp>&& rhs) {
    using _vec = glm::vec<3, IntegralType, glm::defaultp>;

    _vec res = static_cast<_vec>(lhs) + rhs;

    lhs = static_cast<hvox::BlockChunkPosition>(res);

    return lhs;
}
