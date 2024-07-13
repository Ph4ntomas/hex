#ifndef hex_mpi_commands_hpp_
#define hex_mpi_commands_hpp_

#include <hex/commands/types.hpp>
#include <hex/commands/registry.hpp>

/**
** Commands Namespace
**
** In Hex::Mpi, commands are synchronous function calls. Whenever a command is
** called, it runs in the same thread it was called from. Because of this, they
** can have a return value.
**
** Commands are simple functions that are associated with a specific type, the
** command's Recipient. This is not necessarily the type of the object that will
** handle the call however. It's just a way for commands to be grouped together,
** and so that every command has a unique signature.
**
** Before calling a Command, it's necessary to register a CommandHandler into
** the \ref hex::commands::registry "copmmand regitry".
*/
namespace hex::commands {}

#endif
