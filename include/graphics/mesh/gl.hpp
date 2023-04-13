#define ACCUMULATE_VERTEX_ATTRIB_OFFSET(VERTEX_INFO, CURR_OFFSET)                      \
  CURR_OFFSET + (VERTEX_ELEM_COUNT VERTEX_INFO * VERTEX_PRECISION VERTEX_INFO / 8)

#define GEN_VERTEX_ATTRIB(PREFIX, VERTEX_INFO, OFFSET)                                 \
  glEnableVertexArrayAttrib(                                                           \
      handles.vao,                                                                     \
      static_cast<GLuint>(PREFIX##_MeshAttribID::VERTEX_ENUM_NAME VERTEX_INFO)         \
  );                                                                                   \
  glVertexArrayAttribFormat(                                                           \
      handles.vao,                                                                     \
      static_cast<GLuint>(PREFIX##_MeshAttribID::VERTEX_ENUM_NAME VERTEX_INFO),        \
      VERTEX_ELEM_COUNT VERTEX_INFO,                                                   \
      VERTEX_PRECISION VERTEX_INFO == 32 ? GL_FLOAT : GL_DOUBLE,                       \
      VERTEX_NORMALISED VERTEX_INFO == 1 ? GL_TRUE : GL_FALSE,                         \
      OFFSET                                                                           \
  );                                                                                   \
  glVertexArrayAttribBinding(                                                          \
      handles.vao,                                                                     \
      static_cast<GLuint>(PREFIX##_MeshAttribID::VERTEX_ENUM_NAME VERTEX_INFO),        \
      0                                                                                \
  );

#define GEN_MESH_UPLOADER_DEF(PREFIX, INDEXED, ...)                                    \
  bool hg::upload_mesh(                                                                \
      IF_ELSE(NOT(INDEXED)                                                             \
      )(const Const##PREFIX##_MeshData&        mesh_data,                              \
        const Const##PREFIX##_IndexedMeshData& mesh_data),                             \
      IF_ELSE(NOT(INDEXED)                                                             \
      )(OUT MeshHandles & handles, OUT IndexedMeshHandles & handles),                  \
      MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/                  \
  ) {                                                                                  \
    assert(mesh_data.vertices != nullptr);                                             \
                                                                                       \
    IF(INDEXED)                                                                        \
      (assert(mesh_data.indices != nullptr);)                                          \
                                                                                       \
          glCreateVertexArrays(1, &handles.vao);                                       \
                                                                                       \
    glCreateBuffers(1, &handles.vbo);                                                  \
    glNamedBufferData(                                                                 \
        handles.vbo,                                                                   \
        sizeof(PREFIX##_Vertex) * mesh_data.vertex_count,                              \
        mesh_data.vertices,                                                            \
        static_cast<GLenum>(volatility)                                                \
    );                                                                                 \
                                                                                       \
    glVertexArrayVertexBuffer(                                                         \
        handles.vao, 0, handles.vbo, 0, sizeof(PREFIX##_Vertex)                        \
    );                                                                                 \
                                                                                       \
    IF(INDEXED)                                                                        \
      (glCreateBuffers(1, &handles.ibo); glNamedBufferData(                            \
           handles.ibo,                                                                \
           sizeof(mesh_data.indices[0]) * mesh_data.index_count,                       \
           mesh_data.indices,                                                          \
           static_cast<GLenum>(volatility)                                             \
      );                                                                               \
                                                                                       \
       glVertexArrayElementBuffer(handles.vao, handles.ibo);)                          \
                                                                                       \
          BIND_MAP_WITH_ACCUMULATE(                                                    \
              GEN_VERTEX_ATTRIB,                                                       \
              PREFIX,                                                                  \
              EMPTY,                                                                   \
              0,                                                                       \
              ACCUMULATE_VERTEX_ATTRIB_OFFSET,                                         \
              __VA_ARGS__                                                              \
          )                                                                            \
                                                                                       \
              return static_cast<bool>(mesh_data.vertex_count);                        \
  }
