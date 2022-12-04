#Command Buffering for Multithreaded Connected Script Environments

## Foreign Function

Any script environment may choose between public and private accessibility. In the
former, all functions are by default accessible to another environment as a foreign
function. In the latter, all functions are by default inaccessible for the same. Such
accessible functions are considered one-time functions, in that a call to such a
function as a foriegn function is simply pending on the call being made, and is
complete on the first time it is pumped from the command buffer.

In either setting of accessibility, dedicated foreign functions may be exposed for
calling by other environments. These have to follow a simple extra step to ordinary
functions as above: their first return value is an integer indicating completion status.
Completion is indicated by a 0, while any other integer indicates the function is not
yet complete, leading to such a foreign function call to be buffered again.

## API

In script environment, we need to expose functions*:
  * call_foreign
  * query_foreign_call
  * get_foreign_call_results
  * set_manual_command_buffer_pump
  * pump_command_buffer

*names subject to change, and need a namespace.

### call_foreign

Signature:
i64 call_foreign(string mod_name, string function_name, Params... params)

Call foreign queues a call to function_name in command buffer of the environment of mod
with name mod_name. The params are bound to this call, and are passed to the called
function at call time.

The return is -1 on failure (TODO: why might we fail? buffer overflow?), and any other
value serves as ID which can be used in querying.

### query_foreign_call

Signature:
i32 query_foreign_call(string mod_name, i64 id)

Queries the state of a buffered foreign call with ID id. Returned value is -1 if no
foreign call is buffered with ID id, and otherwise an enumerated state:
    * 0 - complete
    * 1 - running
    * 2 - pending

### get_foreign_call_results

Signature:
i32, ReturnValues... get_foreign_call_results(string mod_name, i64 id)

Obtains the results of a buffered foreign call with ID id. The first value returned is
a status, -1 being returned if no foreign call is buffered with ID id, and otherwise an
enumerated state:
    * 0 - pending or complete with no return
    * 1 - complete with return
the rest of the return values are those returned by the called foreign function.

### set_manual_command_buffer_pump

Signature:
i32 set_manual_command_buffer_pump(bool use_manual = true)

Calling set_manual_command_buffer_pump with use_manual set to true (the default), sets
the calling environment to not automatically pump its command buffer before the update
function(s)
of that environment is(are) called.Instead, it is assumed that the
    pump_command_buffer function will be called manually at an
    appropriate point in the execution of the update function(s)
of the environment.Calls without                                setting this to true,
    or else subsequent calls in the same update after the first will do nothing.

        ## #pump_command_buffer

        Signature :
    void
    pump_command_buffer()

        On first call in the update of an
    environment which has enabled manual command buffer pumping,
    this will cause all commands buffered since the previous update to be executed in
    turn.
