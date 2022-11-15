#ifndef __hemlock_script_command_buffer_hpp
#define __hemlock_script_command_buffer_hpp

namespace hemlock {
    namespace script {
        // TODO(Matthew): Reduce new calls as much as possible, introduce pageing and
        //                command limits. Try to remove use of std::string for commands,
        //                but not sure how to do that just yet.

        using CommandID = i64;

        using Commands = std::vector<std::pair<CommandID, std::string>>;

        enum class CommandState : ui8 {
            COMPLETE = 0,
            RUNNING  = 1,
            PENDING  = 2
        };

        // TODO(Matthew): Can we support arrays? Tables?
        enum class CommandCallType {
            BOOLEAN,
            NUMBER,
            STRING,
            USERDATA
        };
        using CommandCallValue  = std::byte[8];
        using CommandCallValues = std::vector<
                                        std::pair<
                                            CommandCallType,
                                            CommandCallValue
                                        >
                                   >;

        struct CommandData {
            size_t              index;
            CommandState        state;
            CommandCallValues   call_values;
        };
        using CommandsData = std::unordered_map<CommandID, CommandData>;

        // TODO(Matthew): Speed? Mutex use is heavy.
        template <size_t BufferSize = 0>
        class CommandBuffer {
        public:
            CommandBuffer() :
                m_latest_command_id(0), m_commands_buffered(0)
            { /* Empty. */ }
            ~CommandBuffer() { dispose(); }

            void init();
            void dispose();

            /**
             * @brief Appends the given command to the command buffer
             * if the buffer is not already full.
             *
             * @param command The command to append.
             * @return CommandID The ID associated with the command,
             * negative if the command was not appended. (Currently
             * -1 is returned solely.)
             */
            CommandID append_command(std::string&& command);

            /**
             * @brief Get the state of buffered command with the
             * given ID.
             *
             * @param id The ID of the command to get the state of.
             * @param state The value populated with the state of
             * the buffered command.
             * @return i32 -1 if no command is buffered with
             * given ID, otherwise 0.
             */
            i32 command_state(CommandID id, OUT CommandState& state);

            /**
             * @brief Get the return values of buffered command with
             * the given ID.
             *
             * @param id The ID of the command to get the return values
             * of.
             * @param return_values The return values of the buffered
             * command.
             * @return i32 -1 if no command is buffered with given
             * ID, otherwise 0.
             */
            i32 command_return_values( CommandID id, 
                        OUT CommandReturnValues& return_values );

            /**
             * @brief Removes command with the given ID from the buffer.
             *
             * @param id The ID of the command to remove from the buffer.
             * @return i32 -1 if no command is buffered with given
             * ID, otherwise 0.
             */
            i32 remove_command(CommandID id);
        protected:
            CommandID       m_latest_command_id;
            Commands        m_commands;
            CommandsData    m_command_data;

            std::mutex      m_buffer_lock;

            std::conditional<
                std::greater<size_t>()(BufferSize, 0),
                size_t,
                std::monostate
            >::type m_commands_buffered;
        };
    }
}
namespace hscript = hemlock::script;

#include "command_buffer.inl"

#endif // __hemlock_script_command_buffer_hpp
