# 2. Interrupt Logging Strategy

Date: 2021-04-15

## Status

Accepted

## Context

We received this request:

> We would like to be able to add an entry into the log during interrupt routines.  The current circular buffer works fine for that, however default library behavior to initiates a flush when the buffer is full. This is a problem if we are in an interrupt routine. Perhaps we could put the logger into a state that does not flush. If it should have flushed, we can trigger an overrun condition that would be later logged by the main loop (where we would also control the flush timing).

## Decision

- We will create APIs that can be used to enable/disable the automated `flush` behavior.
- We will create a `log_interrupt()` API that will disable flushing if it is enabled, and re-enabling the flush afterward.
    + This API *will not* force a `flush()` to occur when re-enabling the auto-flush behavior.
- We need to create a method that allows a user to identify when a circular buffer overrun has occurred prior to flushing.

## Consequences

- Users must be responsible for flushing the log buffer outside of the interrupt context
- Because the `log_interrupt()` API does not trigger flush behavior when re-enabling the auto-flush mode, the user risks losing data if there is a non-interrupt log call that is made prior to flushing the buffer.
