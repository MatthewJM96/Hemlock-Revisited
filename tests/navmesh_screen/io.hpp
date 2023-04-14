#ifndef __hemlock_tests_navmesh_screen_io_hpp
#define __hemlock_tests_navmesh_screen_io_hpp

namespace hemlock {
    namespace test {
        namespace navmesh_screen {
            void handle_simple_user_inputs(
                hui::InputManager*            input_manager,
                hcam::BasicFirstPersonCamera& camera,
                f32                           frame_time,
                OUT bool&                     draw_chunk_outlines,
                OUT ui32&                     nav_test_mode,
                OUT f32&                      speed_mult,
                OUT f32v3&                    delta_pos
            ) {
                if (input_manager->is_pressed(hui::PhysicalKey::H_L)) {
                    draw_chunk_outlines = true;
                }

                if (input_manager->is_pressed(hui::PhysicalKey::H_T)) {
                    nav_test_mode = 1;
                }

                speed_mult = 1.0f;
                if (input_manager->key_modifier_state().ctrl) {
                    speed_mult = 10.0f;
                }
                if (input_manager->key_modifier_state().alt) {
                    speed_mult = 50.0f;
                }

                delta_pos = {};
                if (input_manager->is_pressed(hui::PhysicalKey::H_W)) {
                    delta_pos += glm::normalize(camera.direction()) * frame_time * 0.01f
                                 * speed_mult;
                }
                if (input_manager->is_pressed(hui::PhysicalKey::H_A)) {
                    delta_pos -= glm::normalize(camera.right()) * frame_time * 0.01f
                                 * speed_mult;
                }
                if (input_manager->is_pressed(hui::PhysicalKey::H_S)) {
                    delta_pos -= glm::normalize(camera.direction()) * frame_time * 0.01f
                                 * speed_mult;
                }
                if (input_manager->is_pressed(hui::PhysicalKey::H_D)) {
                    delta_pos += glm::normalize(camera.right()) * frame_time * 0.01f
                                 * speed_mult;
                }
                if (input_manager->is_pressed(hui::PhysicalKey::H_Q)) {
                    delta_pos += glm::normalize(camera.up()) * frame_time * 0.01f
                                 * speed_mult;
                }
                if (input_manager->is_pressed(hui::PhysicalKey::H_E)) {
                    delta_pos -= glm::normalize(camera.up()) * frame_time * 0.01f
                                 * speed_mult;
                }
            }
        }  // namespace navmesh_screen
    }      // namespace test
}  // namespace hemlock
namespace htest = hemlock::test;

#endif  // __hemlock_tests_navmesh_screen_io_hpp
