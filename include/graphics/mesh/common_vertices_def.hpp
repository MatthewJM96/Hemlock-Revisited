#define MAKE_POS2D(PRECISION)                                                          \
  (Vertex_Pos2D_##PRECISION, position, POSITION, 2, PRECISION, 0)
#define MAKE_POS3D(PRECISION)                                                          \
  (Vertex_Pos3D_##PRECISION, position, POSITION, 3, PRECISION, 0)
#define MAKE_NORMAL2D(PRECISION)                                                       \
  (Vertex_Normal2D_##PRECISION, normal, NORMAL, 2, PRECISION, 0)
#define MAKE_NORMAL3D(PRECISION)                                                       \
  (Vertex_Normal3D_##PRECISION, normal, NORMAL, 3, PRECISION, 0)
#define MAKE_UV(PRECISION)   (Vertex_UV_##PRECISION, uv, UV, 2, PRECISION, 0)
#define MAKE_RGB(PRECISION)  (Vertex_RGB_##PRECISION, rgb, RGB, 3, PRECISION, 1)
#define MAKE_RGBA(PRECISION) (Vertex_RGBA_##PRECISION, rgba, RGBA, 4, PRECISION, 1)
