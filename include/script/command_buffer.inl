template <size_t BufferSize>
void hscript::CommandBuffer<BufferSize>::init() {
    if constexpr (BufferSize > 0) {
        m_commands.resize(BufferSize);
    }
}

template <size_t BufferSize>
void hscript::CommandBuffer<BufferSize>::dispose() {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    Commands().swap(m_commands);
    CommandsData().swap(m_command_data);

    m_latest_command_id = 0;
    if (BufferSize > 0) m_commands_buffered = 0;
}

template <size_t BufferSize>
hscript::CommandID hscript::CommandBuffer<BufferSize>::append_command(std::string&& command) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    CommandID id  = m_latest_command_id++;
    size_t    idx = 0;

    // Handle command buffering differently depending on if we have an uncapped buffer
    // size, or a fixed buffer size.
    if constexpr (BufferSize == 0) {
        idx = m_commands.size();

        m_commands.emplace_back(std::make_pair(id, std::move(command)));
    } else {
        if (m_commands_buffered < BufferSize) {
            idx = m_commands_buffered;

            m_commands[m_commands_buffered++] = std::make_pair(id, std::move(command));
        } else {
            return -1;
        }
    }

    m_command_data.insert(
        std::make_pair(
            id,
            CommandData{
                idx,
                CommandState::PENDING,
                {}
            }
        )
    );

    return id;
}

template <size_t BufferSize>
i32 hscript::CommandBuffer<BufferSize>::command_state(CommandID id, OUT CommandState& state) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    auto it = m_command_data.find(id);

    if (it == m_command_data.end()) {
        return -1;
    }

    state = it->second.state;

    return 0;
}

template <size_t BufferSize>
i32 hscript::CommandBuffer<BufferSize>::command_return_values( CommandID id,
                                                  OUT CommandCallValues& return_values ) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    auto it = m_command_data.find(id);

    if (it == m_command_data.end()) {
        return -1;
    }

    return_values = it->second.call_values;

    return 0;
}

template <size_t BufferSize>
i32 hscript::CommandBuffer<BufferSize>::remove_command(CommandID id) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    auto it = m_command_data.find(id);

    if (it == m_command_data.end()) {
        return -1;
    }

    auto idx = it->second.index;

    if constexpr (BufferSize == 0) {
        if (idx != m_commands.size() - 1) {
            m_commands[idx] = std::move(m_commands[m_commands.size() - 1]);
        }
        m_commands.pop_back();
    } else {
        if (idx != m_commands_buffered - 1) {
            m_commands[idx] = std::move(m_commands[m_commands_buffered - 1]);
        }
        m_commands_buffered -= 1;
    }

    m_command_data.erase(it);
}
