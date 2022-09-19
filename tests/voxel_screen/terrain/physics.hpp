#ifndef __hemlock_tests_voxel_screen_terrain_physics_hpp
#define __hemlock_tests_voxel_screen_terrain_physics_hpp

namespace hemlock {
    namespace test {
        namespace voxel_screen {
            struct TVS_VoxelShapeEvaluator {
                btCollisionShape* operator()(hvox::Block b, btTransform&) const {
                    if (b == hvox::Block{1}) {
                        return new btBoxShape(btVector3{0.5f, 0.5f, 0.5f});
                    }
                    return nullptr;
                }
            };

            class VoxelPhysDrawer : public btIDebugDraw {
            public:
                VoxelPhysDrawer(hcam::BasicFirstPersonCamera* camera, hg::GLSLProgram* shader) : btIDebugDraw() {
                    m_camera = camera;
                    m_line_shader = shader;

                    glCreateVertexArrays(1, &m_vao);

                    glCreateBuffers(1, &m_vbo);
                    glNamedBufferData(
                        m_vbo,
                        sizeof(f32v3) * 2,
                        nullptr,
                        GL_DYNAMIC_DRAW
                    );

                    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(f32v3));

                    glEnableVertexArrayAttrib(m_vao, 0);
                    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
                    glVertexArrayAttribBinding(m_vao, 0, 0);
                }
                virtual ~VoxelPhysDrawer() { /* Empty. */ }

                virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3&) override {
                    f32v3 points[2] = {
                        f32v3{
                            from.x()/* + m_camera->position().x*/,
                            from.y()/* + m_camera->position().y*/,
                            from.z()/* + m_camera->position().z*/
                        },
                        f32v3{
                            to.x()/* + m_camera->position().x*/,
                            to.y()/* + m_camera->position().y*/,
                            to.z()/* + m_camera->position().z*/
                        }
                    };

                    glNamedBufferSubData(m_vbo, 0, 2 * sizeof(f32v3), reinterpret_cast<void*>(&points[0]));

                    f32v3 _colour = f32v3{1.0f, 1.0f, 0.0f};
                    glUniformMatrix4fv(m_line_shader->uniform_location("view_proj"),  1, GL_FALSE, &m_camera->view_projection_matrix()[0][0]);
                    glUniform3fv(m_line_shader->uniform_location("colour"), 1, &_colour[0]);

                    glBindVertexArray(m_vao);
                    glDrawArrays(GL_LINES, 0, 2);
                }

                virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override { /* Empty. */ }

                virtual void reportErrorWarning(const char* warning_string [[maybe_unused]]) override { debug_printf(warning_string); }

                virtual void draw3dText(const btVector3&, const char*) override { /* Empty. */ }

                virtual void setDebugMode(int debug_mode) override { m_debug_mode = debug_mode; }

                virtual int getDebugMode() const override { return m_debug_mode; }
            protected:
                int m_debug_mode;
                GLuint m_vao, m_vbo;
                hcam::BasicFirstPersonCamera* m_camera;
                hg::GLSLProgram* m_line_shader;
            };
        }
    }
}
namespace htest = hemlock::test;

#endif // __hemlock_tests_voxel_screen_terrain_physics_hpp
