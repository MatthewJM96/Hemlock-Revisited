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
        using CommandsStates = std::unordered_map<CommandID, CommandState>;

        // TODO(Matthew): Can we support arrays? Tables?
        template <std::signed_integral Underlying>
        enum class CommandReturnType : Underlying {
            BOOLEAN,
            NUMBER,
            STRING,
            USERDATA
        };
        using CommandReturnValue   = std::byte[8];
        template <std::signed_integral Underlying>
        using CommandReturnValues  = std::vector<
                                            std::pair<
                                                CommandReturnType<Underlying>,
                                                CommandReturnValue
                                            >
                                        >;
        template <std::signed_integral Underlying>
        using CommandsReturnValues = std::unordered_map<
                                        CommandID,
                                        CommandReturnValues<Underlying>
                                    >;

        template <std::signed_integral NumberType>
        class CommandBuffer {
        public:
            CommandBuffer() :
                m_latest_command_id(0)
            { /* Empty. */ }
            ~CommandBuffer() { /* Empty. */ }

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
             * @return NumberType -1 if no command is buffered with
             * given ID, otherwise 0.
             */
            NumberType command_state(CommandID id, OUT CommandState& state);

            /**
             * @brief Get the return values of buffered command with
             * the given ID.
             *
             * @param id The ID of the command to get the return values
             * of.
             * @param return_values The return values of the buffered
             * command.
             * @return NumberType -1 if no command is buffered with given
             * ID, otherwise 0.
             */
            NumberType command_return_values(CommandID id, OUT CommandReturnValues<NumberType>& return_value);

            /**
             * @brief Removes command with the given ID from the buffer.
             *
             * @param id The ID of the command to remove from the buffer.
             * @return NumberType -1 if no command is buffered with given
             * ID, otherwise 0.
             */
            NumberType remove_command(CommandID id);
        protected:
            CommandID                           m_latest_command_id;
            Commands                            m_commands;
            CommandsStates                      m_command_states;
            CommandsReturnValues<NumberType>    m_return_values;

            std::mutex                          m_command_append_lock;
        };
    }
}
namespace hscript = hemlock::script;

#include "command_buffer.inl"

#endif // __hemlock_script_command_buffer_hpp
