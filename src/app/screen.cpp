#include "stdafx.h"

#include "app/screen.h"

void happ::ScreenBase::init(std::string name, AppBase* app) {
    if (m_initialised) return;
    m_initialised = true;

    m_name = name;
    m_app  = app;
}

void happ::ScreenBase::dispose() {
    if (!m_initialised) return;
    m_initialised = false;
}

void happ::ScreenBase::start(TimeData time [[maybe_unused]]) {
    m_state = ScreenState::RUNNING;
}

void happ::ScreenBase::end(TimeData time [[maybe_unused]]) {
    m_state = ScreenState::NONE;
}
