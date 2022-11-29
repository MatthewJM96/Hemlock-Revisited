template <typename EnvironmentImpl, size_t BufferSize>
void hscript::RPCManager<EnvironmentImpl, BufferSize>::init(_Environment* env, bool is_public /*= true*/) {
    if constexpr (BufferSize > 0) {
        m_calls.resize(BufferSize);
    }

    m_environment   = env;
    m_is_public_env = is_public;
}

template <typename EnvironmentImpl, size_t BufferSize>
void hscript::RPCManager<EnvironmentImpl, BufferSize>::dispose() {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    m_environment = nullptr;

    Calls().swap(m_calls);
    CallsData().swap(m_call_data);

    m_latest_call_id = 0;
    if (BufferSize > 0) m_calls_buffered = 0;
}

template <typename EnvironmentImpl, size_t BufferSize>
hscript::CallID hscript::RPCManager<EnvironmentImpl, BufferSize>::append_call(std::string&& call, CallParameters&& parameters) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    CallID id  = m_latest_call_id++;
    i32    idx = -1;

    // Handle call buffering differently depending on if we have an uncapped buffer
    // size, or a fixed buffer size.
    if constexpr (BufferSize == 0) {
        idx = m_calls.size();

        m_calls.emplace_back(std::make_pair(id, std::move(call)));
    } else {
        if (m_calls_buffered < BufferSize) {
            idx = m_calls_buffered;

            m_calls[m_calls_buffered++] = std::make_pair(id, std::move(call));
        } else {
            return -1;
        }
    }

    m_call_data.insert(
        std::make_pair(
            id,
            CallData{
                idx,
                CallState::PENDING,
                std::move(parameters)
            }
        )
    );

    return id;
}

template <typename EnvironmentImpl, size_t BufferSize>
i32 hscript::RPCManager<EnvironmentImpl, BufferSize>::call_state(CallID id, OUT CallState& state) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    auto it = m_call_data.find(id);

    if (it == m_call_data.end()) {
        return -1;
    }

    state = it->second.state;

    return 0;
}

template <typename EnvironmentImpl, size_t BufferSize>
i32 hscript::RPCManager<EnvironmentImpl, BufferSize>::call_return_values(
             CallID id,
    OUT CallParameters& return_values
) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    auto it = m_call_data.find(id);

    if (it == m_call_data.end()) {
        return -1;
    }

    if (it->second.state != CallState::COMPLETE) {
        return -2;
    }

    return_values = it->second.call_values;

    return 0;
}



template <typename EnvironmentImpl, size_t BufferSize>
i32 hscript::RPCManager<EnvironmentImpl, BufferSize>::remove_call(CallID id) {
    std::lock_guard<std::mutex> lock(m_buffer_lock);

    auto it = m_call_data.find(id);

    if (it == m_call_data.end()) {
        return -1;
    }

    auto idx = it->second.index;

    if constexpr (BufferSize == 0) {
        if (idx != m_calls.size() - 1) {
            m_calls[idx] = std::move(m_calls[m_calls.size() - 1]);
        }
        m_calls.pop_back();
    } else {
        if (idx != m_calls_buffered - 1) {
            m_calls[idx] = std::move(m_calls[m_calls_buffered - 1]);
        }
        m_calls_buffered -= 1;
    }

    m_call_data.erase(it);
}

template <typename EnvironmentImpl, size_t BufferSize>
void hscript::RPCManager<EnvironmentImpl, BufferSize>::pump_calls() {
    // Reorganise a bit to allow for only running so many calls based on demand?
    for (auto& call : m_calls) {
        CallID id = call.first;
        std::string cmd = call.second;

        auto delegate = {};
        m_environment->template get_script_function<CallParameters, CallParameters>(cmd, delegate);

        CallData& call_data = m_call_data[id];

        CallParameters return_values = delegate(m_call_data[id].call_values);

        call_data.state = CallState::COMPLETE;
        call_data.call_values = std::move(return_values);
    }

    // Clear call buffer.
    Calls().swap(m_calls);
}
