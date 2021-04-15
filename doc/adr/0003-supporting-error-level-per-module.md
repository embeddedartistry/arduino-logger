# 3. Supporting Error Level Per-Module

Date: 2021-04-15

## Status

Accepted

## Context

We received this request:

> We have multiple major subsystems and if we turn on Debugging level for all of them the data becomes overwhelming and pretty useless.  We could give each subsystem its own log file but then correlating events across them becomes a problem.  Would be interested in an enhancement where logging level could be set by subsystem.

The challenge here is that we need to support two cases: people who want to specify a module, and people who don't. This causes two scenarios: *everyone* has to care about module IDs, we have an explosion of interfaces, or we introduce lots of template code. Templates would be an ideal approach because we can hide complexity, but this also introduces a requirement that all users deal with it.

Another approach would be to create a strategy that encapsulates this behavior. This might be better than global complexity increases because it is isolated into a single strategy, rather than requiring all strategies to opt-in.

## Decision

We will create a new strategy based on the Teensy SD Rotational Log Strategy that implements multiple-module logging. This will serve as an example for users, while minimizing global complexity increase for the base class.

## Consequences

The requirement is that there is a GLOBAL minimum for log levels, since we are not modifying the base class. 

The general approach will have to be copied to other logging strategies if desired, which introduces additional burdens.

Currently, the APIs for per-module logging are not available via the macros or the global instance class.
