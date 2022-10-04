#ifndef __hemlock_tests_test_entry_screen_hpp
#define __hemlock_tests_test_entry_screen_hpp

#include "app/screen_base.h"
#include "graphics/font/font.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/batcher.h"
#include "ui/input/dispatcher.h"

#include "iomanager.hpp"

class TestEntryScreen : public happ::ScreenBase {
public:
    TestEntryScreen() :
        happ::ScreenBase()
    { /* Empty. */ }
    virtual ~TestEntryScreen() { /* Empty */ };

    virtual void update(hemlock::FrameTime) override {
        // Empty.
    }
    virtual void draw(hemlock::FrameTime) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        happ::WindowDimensions dims = m_process->window()->dimensions();
        m_sprite_batcher.render(f32v2{dims.width, dims.height});
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        handle_key_down = hemlock::Subscriber<hui::KeyboardButtonEvent>{
            [&](hemlock::Sender, hui::KeyboardButtonEvent ev) {
                switch (ev.physical_key) {
                case hui::PhysicalKey::H_R:
                    m_process->go_to_screen("test_render_screen", m_process->timer()->frame_times().back());
                    return;
                case hui::PhysicalKey::H_V:
                    m_process->go_to_screen("test_voxel_screen", m_process->timer()->frame_times().back());
                    return;
                case hui::PhysicalKey::H_S:
                    m_process->go_to_screen("test_script_screen", m_process->timer()->frame_times().back());
                    return;
                default:
                    break;
                }
            }
        };

        hui::InputDispatcher* dispatcher = hui::InputDispatcher::instance();
        dispatcher->on_keyboard.button_down += &handle_key_down;

        m_shader_cache.init(&m_iom, hg::ShaderCache::Parser{
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                std::string buffer;
                if (!iom->read_file_to_string(path, buffer)) return "";

                return buffer;
            }
        });

        m_font_cache.init(&m_iom, hg::f::FontCache::Parser{
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> hg::f::Font {
                hio::fs::path actual_path;
                if (!iom->resolve_path(path, actual_path)) return hg::f::Font{};

                hg::f::Font font;
                font.init(actual_path.string());

                return font;
            }
        });

        auto font = m_font_cache.fetch("fonts/Orbitron-Regular.ttf");
        font->set_default_size(22);
        font->generate();

        m_sprite_batcher.init(&m_shader_cache, &m_font_cache);
        m_sprite_batcher.begin();
        m_sprite_batcher.add_string(
            "Test render screen (R)",
            f32v4{30.0f, 30.0f, 1000.0f, 100.0f},
            f32v4{25.0f, 25.0f, 1010.0f, 110.0f},
            hg::f::StringSizing{hg::f::StringSizingKind::SCALED, {f32v2{1.0f}}},
            colour4{0, 0, 0, 255},
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.add_string(
            "Test voxel screen (V)",
            f32v4{30.0f, 60.0f, 1000.0f, 100.0f},
            f32v4{25.0f, 55.0f, 1010.0f, 110.0f},
            hg::f::StringSizing{hg::f::StringSizingKind::SCALED, {f32v2{1.0f}}},
            colour4{0, 0, 0, 255},
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.add_string(
            "Test script screen (S)",
            f32v4{30.0f, 90.0f, 1000.0f, 100.0f},
            f32v4{25.0f, 85.0f, 1010.0f, 110.0f},
            hg::f::StringSizing{hg::f::StringSizingKind::SCALED, {f32v2{1.0f}}},
            colour4{0, 0, 0, 255},
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE
        );
        m_sprite_batcher.end();
    }
protected:
    hemlock::Subscriber<hui::KeyboardButtonEvent> handle_key_down;

    ui32 m_default_texture;

    MyIOManager          m_iom;
    hg::ShaderCache      m_shader_cache;
    hg::f::FontCache     m_font_cache;
    hg::s::SpriteBatcher m_sprite_batcher;
};

#endif // __hemlock_tests_test_entry_screen_hpp
