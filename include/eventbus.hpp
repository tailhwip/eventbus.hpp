// MIT License
//
// Copyright (c) 2025 https://github.com/tailhwip/eventbus
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

namespace EB
{

template <typename E>
static inline const void *TypeId()
{
    static char dummy;
    return &dummy;
}

// ------------
// EventHandler
// ------------------------------------------------------------------
// Internal representation of an event handler. Consumers of the public
// API do not interact with this interface. Instead add event handling
// functions and or methods directly on the EventBus object.

template <typename E>
class EventHandler
{
public:
    void *object = nullptr;
    void (*proxy)(void *, const E &) = nullptr;

    void operator()(const E &event);

    bool operator==(const EventHandler &other) const;
};

template <typename E>
void EventHandler<E>::operator()(const E &event)
{
    proxy(object, event);
}

template <typename E>
bool EventHandler<E>::operator==(const EventHandler &other) const
{
    return object == other.object && proxy == other.proxy;
}

template <typename E, void (*function)(const E &)>
static void FunctionProxy(void *, const E &event)
{
    function(event);
}

template <typename E, void (*function)(const E &)>
static EventHandler<E> EventHandlerFunction()
{
    return { nullptr, &FunctionProxy<E, function> };
}

template <typename E, typename T, void (T::*method)(const E &)>
static void MethodProxy(void *object, const E &event)
{
    (static_cast<T *>(object)->*method)(event);
}

template <typename E, auto method, typename T>
requires std::same_as<decltype(method), void (T::*)(const E &)>
static EventHandler<E> EventHandlerMethod(T *object)
{
    return { object, &MethodProxy<E, T, method> };
}

// --------------------
// EventHandlerRegistry
// ------------------------------------------------------------------
// Internal storage of event handlers sorted by event type. Consumers
// of the public API do not interact with this interface. Instead publish
// your event directly on the EventBus object.

struct ErasedEventHandlers
{
};

template <typename E>
struct EventHandlersWrapper : public ErasedEventHandlers
{
    std::vector<EventHandler<E>> handlers;
};

class EventHandlerRegistry
{
public:
    template <typename E>
    std::vector<EventHandler<E>> &GetEventHandlers()
    {
        auto &erased = handlers[TypeId<E>()];
        if (!erased)
        {
            erased = std::make_unique<EventHandlersWrapper<E>>();
        }

        return static_cast<EventHandlersWrapper<E> *>(erased.get())->handlers;
    }

private:
    std::unordered_map<const void *, std::unique_ptr<ErasedEventHandlers>> handlers;
};

// --------
// EventBus
// ------------------------------------------------------------------

class EventBus
{
public:
    template <typename E, void (*function)(const E &)>
    void AddHandler();

    template <typename E, auto method, typename T>
    requires std::same_as<decltype(method), void (T::*)(const E &)>
    void AddHandler(T *object);

    template <typename E, void (*function)(const E &)>
    void RemoveHandler();

    template <typename E, auto method, typename T>
    requires std::same_as<decltype(method), void (T::*)(const E &)>
    void RemoveHandler(T *object);

    template <typename E>
    void PublishEvent(const E &event);

private:
    EventHandlerRegistry registry;
};

template <typename E, void (*function)(const E &)>
void EventBus::AddHandler()
{
    auto handler = EventHandlerFunction<E, function>();
    registry.GetEventHandlers<E>().push_back(handler);
}

template <typename E, auto method, typename T>
requires std::same_as<decltype(method), void (T::*)(const E &)>
void EventBus::AddHandler(T *object)
{
    auto handler = EventHandlerMethod<E, method>(object);
    registry.GetEventHandlers<E>().push_back(handler);
}

template <typename E, void (*function)(const E &)>
void EventBus::RemoveHandler()
{
    auto handler = EventHandlerFunction<E, function>();
    std::vector<EventHandler<E>> &it = registry.GetEventHandlers<E>();
    it.erase(std::remove(it.begin(), it.end(), handler), it.end());
}

template <typename E, auto method, typename T>
requires std::same_as<decltype(method), void (T::*)(const E &)>
void EventBus::RemoveHandler(T *object)
{
    auto handler = EventHandlerMethod<E, method>(object);
    std::vector<EventHandler<E>> &it = registry.GetEventHandlers<E>();
    it.erase(std::remove(it.begin(), it.end(), handler), it.end());
}

template <typename E>
void EventBus::PublishEvent(const E &event)
{
    for (auto &handler : registry.GetEventHandlers<E>())
    {
        handler(event);
    }
}

} // namespace EB
