# EventBus

Header‑only C++20 event bus for lightweight, synchronous event dispatch to registered handlers.

## Usage

Initialize an event bus. Note that each event bus maintains its own registry; make sure that application components that depend on each other are using the same instance.

```cpp
EB::EventBus events;
```

Register a handler for a specific event type. Events can be anything and do not need to adhere to any interface. You can register both functions as well as object methods as event handlers. An event type can have multiple handlers registered.

Passing each handler as a non-type template parameter ensures that all handler calls are statically bound and can be fully inlined by the compiler. This eliminates dynamic dispatch and runtime lookups, leaving only a single pointer indirection from the handler registry when invoking each handler.

```cpp
// registering a function

struct CollisionEvent
{
    float x, y;
};

void collision_handler(CollisionEvent const &)
{
}

events.AddHandler<CollisionEvent, collision_handler>();

// registering an object method

struct CollisionHandler
{
    void on_event(CollisionEvent const &)
    {
    }
};

CollisionHandler handler;

events.AddHandler<CollisionEvent, &CollisionHandler::on_event>(&handler);
```

Publish an event by creating an object of the type your handlers are expecting and pass it to the bus. Currently handlers can only accept a const reference to this object.

```cpp
CollisionEvent e;

events.PublishEvent(e);
```

## Improvements

- Add support for returning data from event handlers to the publisher to enable cancellable events.
