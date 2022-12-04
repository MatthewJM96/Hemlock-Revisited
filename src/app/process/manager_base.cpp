#include "stdafx.h"

#include "app/process/process_base.h"

#include "app/process/manager_base.h"

happ::ProcessManagerBase::ProcessManagerBase() :
    m_initialised(false), m_main_process(nullptr), m_app(nullptr){ /* Empty. */ };

void happ::ProcessManagerBase::init(AppBase* app) {
    if (m_initialised) return;
    m_initialised = true;

    m_app = app;
}

void happ::ProcessManagerBase::dispose() {
    if (!m_initialised) return;
    m_initialised = false;

    end_processes();

    m_main_process = nullptr;
    for (auto& process : m_processes) {
        process.second->dispose();
        delete process.second;
    }
    Processes().swap(m_processes);

    m_app = nullptr;
}

happ::ProcessBase* happ::ProcessManagerBase::process(ui32 id) {
    auto it = std::find_if(m_processes.begin(), m_processes.end(), [id](auto& rhs) {
        return rhs.first == id;
    });
    if (it == m_processes.end()) return nullptr;

    return (*it).second;
}

void happ::ProcessManagerBase::end_process(ui32 id) {
    auto it = std::find_if(m_processes.begin(), m_processes.end(), [id](auto& rhs) {
        return rhs.first == id;
    });
    if (it == m_processes.end()) return;

    (*it).second->set_should_end_process();
}
