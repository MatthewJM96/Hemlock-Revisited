#include "stdafx.h"

#include "app/screen.h"

void happ::IScreen::init(std::string name) {
    if (m_initialised) return;
    m_initialised = true;

    m_name = name;
}

void happ::IScreen::dispose() {
    if (!m_initialised) return;
    m_initialised = false;
}

void happ::IScreen::start(TimeData time [[maybe_unused]]) {
    m_state = ScreenState::RUNNING;
}

void happ::IScreen::end(TimeData time [[maybe_unused]]) {
    m_state = ScreenState::NONE;
}
