#include "stdafx.h"

#include "app/screen_base.h"

void happ::ScreenBase::init(const std::string& name, ProcessBase* process) {
    if (m_initialised) return;
    m_initialised = true;

    m_name    = name;
    m_process = process;
}

void happ::ScreenBase::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    m_process = nullptr;

    m_next_screen = nullptr;
    m_prev_screen = nullptr;

    m_state = ScreenState::NONE;
}

void happ::ScreenBase::start(TimeData) {
    m_state = ScreenState::RUNNING;
}

void happ::ScreenBase::end(TimeData) {
    m_state = ScreenState::NONE;
}
