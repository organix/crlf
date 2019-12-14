# ABCM - Actor Byte-Code Machine

The **Actor Byte-Code Machine** is a byte-code interpreter for _Actor_ programs.
Programs are stored and executed directly from Binary Octet-Stream Encoding representation (isomorphic to JSON).

## Actor Assignment-Machine Model

An Actor's behavior can be described using a sequential model of variable assignments.
This is essentially the model of contemporary processor cores.
However, in order to avoid the pitfalls of shared mutable state,
we only allow assignment to actor-local (private) variables.
All visible effects are captured in the asynchronous messages between actors.

A _Configuration_ is a collection of _Actors_ and (pending) _Events_.
It represents the stable internal "state" of an Actor system between message deliveries.

A _Sponsor_ plays the role of a processor core,
mediating access to computational resources
and executing the instructions in an Actor's behavior script (the _program_).
Each message delivery event is handled as if it was an atomic transaction.
No effects are visible outside the Actor until message handling is completed.
Message handling may be aborted by an exception,
in which case the message is effectively ignored.

A _Dictionary_ mapping names to values is the primary conceptual data structure.
Each Actor maintains a persistent Dictionary of local variables, representing it's private state.
Each message is a read-only _Dictionary_ from which values may be retrieved by the Actor's behavior script.
Information derived from the message may be assigned to the Actor's local persistent state.

## BART Program Elements

ABCM programs are represented in a _CRLF_-style _JSON_ vocabulary called _BART_ (for **Blockly Actor Run-Time**).
[BART](https://github.com/dalnefre/blockly/tree/BART) is an implementation of an Actor Assignment Machine using the [Blockly](https://developers.google.com/blockly/) web-based visual programming editor. BART programs are encoded as [BOSE](BOSE.md) representations of JSON values. The top-level value is expected to be an Array of Sponsors.

An Actor program is a sequence of _Actions_ executing within the bounds of a _Sponsor_:
```javascript
// Containers
    { "kind":"actor_sponsor", "actors":<number>, "events":<number>, "script":[<action>, ...] }
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
    { "kind":"conditional", "args":[{ "if":<expression>, "do":[<action>, ...] }, ...] }
    { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
```

Typed expressions include:
```javascript
// Address Expressions
    { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
    { "kind":"actor_self" }
// Behavior Expressions
    { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
// Value Expressions
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"expr_literal", "const":<value> }
    { "kind":"expr_operation", "name":<string>, "args":[<expression>, ...] }
// Number Expressions
    { "kind":"list_length", "of":<list> }
// Boolean Expressions
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
// List (Array) Expressions
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "value":<expression>, "at":<number>, "from":<list> }
// Dictionary (Object) Expressions
    { "kind":"actor_message" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
```

Primitive operations include:
```javascript
// String operations
    { "kind":"string_length", "string":<string> }
    { "kind":"string_at", "index":<number>, "string":<string> }
    { "kind":"string_insert", "index":<number>, "value":<number>, "string":<string> }
// Array operations
    { "kind":"array_length", "array":<array> }
    { "kind":"array_at", "index":<number>, "array":<array> }
    { "kind":"array_insert", "index":<number>, "value":<expression>, "array":<array> }
```

## Resource Management

ABCM programs execute within a strictly confined context, controlled by a _Sponsor_.
All resources available to a computation are provided by and mediated by the Sponsor.
At a minimum, this appears in the form of limits on `actors` and `events`.
If a computation attempts to create more `actors` than specified,
or sends more than `events` messages, the computation will be halted.
_A means for requesting more resources is needed, but not yet defined._

### Memory

During the execution of an ABCM program, additional working storage is often required.
Various policies may apply to storage requests, based on their purpose and intent.
Memory management strategies include:
  * _Static_: Built in to executable image. No `reserve` or `release`.
  * _Precise_: Explicitly paired `reserve` and `release` for each allocation.
  * _Reference-Counted_: Like _Precise_ but `retain` creates alias for sharing.
  * _Linear_: Only `reserve`. All allocation reclaimed at end of transaction.
  * _Garbage-Collected_: Only `reserve`. Periodically reclaim orphans not connected to root object(s).
  * _Managed_: Subordinate allocations within another object with custom policies.

When an _Actor_ is created, it is likely to persist beyond the end of a particular computation.
Actors may hold references to other Actors, potentially forming circular reference graphs.
Since Actor-state is mutable, the set of references it holds may change over time.
Memory for an Actor can be reclaimed when it is no longer reachable,
either through other Actors or from a pending message.

When a _Message_ is created, it persists beyond the end of the computation, unless the computation throws.
Asynchronous messages created (sent, but not yet received) are a primary _effect_ of an Actor's behavior.
Memory for a Message can be reclaimed once the message has been delivered and processed by the target Actor.

Temporary working storage may be needed during Actor message processing.
The amount of memory available is not directly specified, but is controlled by the Sponsor.
Temporary memory can be reclaimed when an Actor finishes processing a message (or throws).

An Actor's state persists between message deliveries.
It can be reclaimed when the Actor is reclaimed.
Updates to the Actor's persistent state may copy information from temporary working storage, allowing it to persist.

### Structures

Ownership of resources, especially memory, must be carefully managed.
All computations must be provided a mechanism to access the resources they need.

#### Pool
A pool of persistent managed storage.

Fields:
  * _Capacity_: Number of `BYTE`s available

Methods:
  * _Reserve_: Allocate a number of usable `BYTE`s
  * _Copy_: Make a copy of a Value, possibly from a different pool
  * _Retain_: Make an alias for a shared Value
  * _Release_: Mark an allocation for reclamation

#### Sponsor
A root object, providing access to resource-management mechanisms for computations.

Fields:
  * _Pool_: Working-memory pool
  * _Configuration_: A collection of _Actors_ and _Events_
  * _Event_: Current message-delivery event

Methods:
  * _(none)_

#### Configuration
A collection of _Actors_ and (pending) _Events_.

Fields:
  * _Actors_: All Actors contained in this Configuration
  * _Events_: Pending message-delivery Events
  * _Limits_: Configuration resource limits

Methods:
  * _Dispatch_: Attempt to deliver a pending _Event_
  * _Commit_: Apply message-delivery Effects
  * _Rollback_: Abort message-delivery and clean up Effects

#### Event
An Actor message-delivery Event.

Fields
  * _Actor_: Target of the current event
  * _Message_: Message content (Object)
  * _Effect_: Message-delivery outcome

Methods:
  * _(TBD)_

### Effect
Effects of Actor Commands triggered by a message-delivery Event.

Fields:
  * _Actors_: New Actors created
  * _Events_: New message-delivery Events
  * _Behavior_: Actor's behavior for subsequent messages
  * _State_: Actor's State updates
  * _Error_: Error value, or `null` if none

Methods:
  * _Create_: Create a new Actor
  * _Send_: Send an asynchronous Message
  * _Become_: Define subsequent Actor Behavior
  * _Assign_: Define subsequent Actor State
  * _Fail_: Signal an Error
