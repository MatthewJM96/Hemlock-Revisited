#include "stdafx.h"

#include "script/command_buffer.hpp"

hscript::CommandBufferIterator::CommandBufferIterator(CommandsData::iterator& it, std::mutex& mutex) {
    m_data_iterator = it;
    m_mutex         = &mutex;

    m_mutex->lock();
}

hscript::CommandBufferIterator::~CommandBufferIterator() {
    if (m_mutex)
        m_mutex->unlock();
}

hscript::CommandBufferIterator& hscript::CommandBufferIterator::operator=(const CommandBufferIterator& rhs) {
    m_mutex         = rhs.m_mutex;
    m_data_iterator = rhs.m_data_iterator;

    return *this;
}

hscript::CommandBufferIterator& hscript::CommandBufferIterator::operator=(CommandBufferIterator&& rhs) {
    m_mutex         = rhs.m_mutex;
    m_data_iterator = std::move(rhs.m_data_iterator);

    rhs.m_mutex = nullptr;

    return *this;
}

hscript::CommandBufferIterator& hscript::CommandBufferIterator::operator++() {
    ++m_data_iterator;

    return *this;
}
