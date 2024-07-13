#ifndef hex_mpi_events_hpp_
#define hex_mpi_events_hpp_

#include <hex/events/dispatcher.hpp>
#include <hex/events/callbacks.hpp>
#include <hex/events/queues.hpp>
#include <hex/events/utils.hpp>

/**
** \brief Events Namespace
**
** Hex::Mpi events are based on a central \ref dispatcher, to which other systems
** send the events, to be handled asynchronously.
**
** There are currently two flavor of event handling: callback based and polling
** based.
**
** ### Callback based events
** When callback based events are dispatched, every callback is called with
** a copy of the event.
**
** #### Callback dispatch policy
** There are currently three
** \ref hex::events::callbacks::dispatch_policy "policies" for dispatching
** events:
** - \ref hex::events::callbacks::dispatch_policy::sync "Synchronous":
**   The event is dispatched in the calling thread.
** - \ref hex::events::callbacks::dispatch_policy::async "Asynchronous":
**   The event is dispatched as if in a different thread
** - \ref hex::events::callbacks::dispatch_policy::trigger "Trigger based":
**   The events will be dispatched then triggered, in the triggering thread.
**
** ### Polling base events
** Polling based events are managed via a queue, that can be polled, either to
** retrieve one event, or every pending events. The queue should be cleared
** regularly, so it doesn't overload the memory.
** \warning Queue are never cleared automatically by the dispatcher.
*/
namespace hex::events {
}

#endif /* end of include guard: hex_mpi_events_hpp_ */
