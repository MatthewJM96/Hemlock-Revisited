#include "stdafx.h"

#include "mod/environment.h"

H_DEF_STRUCT_WITH_SERIALISATION(
    hemlock::mod,
    LoadOrder,
    (_load_order_version, ui16),
    (_reserved, ui16),
    (id, hmod::LoadOrderID),
    (name, std::string),
    (mods, std::vector<hmod::ModID>),
    (description, std::string)
    // Last updated?
    // Version?
)

hmod::ModEnvironment::ModEnvironment() :
    m_is_active(false), m_base_iom(nullptr) { /* Empty. */
}

void hmod::ModEnvironment::init(ModRegistry&& registry) {
    m_registry = std::forward<ModRegistry>(registry);
}

void hmod::ModEnvironment::init(ModRegistry&& registry, LoadOrder&& load_order) {
    m_registry   = std::forward<ModRegistry>(registry);
    m_load_order = std::forward<LoadOrder>(load_order);
}

void hmod::ModEnvironment::dispose() {
    deactivate_environment();

    m_registry   = {};
    m_load_order = {};

    std::vector<Mod>().swap(m_mods);
}

void hmod::ModEnvironment::activate_environment() { }

void hmod::ModEnvironment::deactivate_environment() { }

void hmod::ModEnvironment::update(FrameTime time) { }

bool hmod::ModEnvironment::set_load_order(LoadOrder&& load_order) {
    if (m_is_active) return false;

    m_load_order = std::forward<LoadOrder>(load_order);

    return true;
}
