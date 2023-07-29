#include "stdafx.h"

#include "mod/environment.h"

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
    deactivate();

    m_registry   = {};
    m_load_order = {};

    std::vector<Mod>().swap(m_mods);
}

void hmod::ModEnvironment::activate() {
    // Load each mod's metadata.

    // Validate load order.

    // For each mod, run load sequence (TBD)
    //   Load in any static data that should be preloaded.
}

void hmod::ModEnvironment::deactivate() { }

void hmod::ModEnvironment::update(FrameTime time [[maybe_unused]]) { }

bool hmod::ModEnvironment::set_load_order(LoadOrder&& load_order) {
    if (m_is_active) return false;

    m_load_order = std::forward<LoadOrder>(load_order);

    return true;
}
