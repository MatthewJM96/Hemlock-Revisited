#include "stdafx.h"

#include "app/process/manager_base.h"

#include "app/app_base.h"

happ::AppBase::AppBase()  :
    handle_external_quit(Subscriber<>{[&](Sender) {
        set_should_quit();
    }}),
    m_process_manager(nullptr)
{ /* Empty */ }

void happ::AppBase::set_should_quit(bool should_quit /*= true*/) {
    m_should_quit = should_quit;
    if (should_quit) on_quit();
}

void happ::AppBase::quit() {
    m_process_manager->dispose();

#if defined(HEMLOCK_USING_SDL)
    SDL_Quit();
#endif // defined(HEMLOCK_USING_SDL)
    exit(0);
}
