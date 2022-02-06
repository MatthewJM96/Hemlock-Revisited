#ifndef __hemlock_event_h
#define __hemlock_event_h

namespace hemlock {
    // Sender is the object responsible for handling the event, and is stored
    // simply as a void pointer; making things easier for us and harder for
    // users to do stupid things.
    using Sender = const void*;

    /**
     * @brief Event base class, provides all aspects of the Event class that don't
     * depend on templated parameters.
     */
    class EventBase {
    public:
        /**
        * @brief Default constructor
        *
        * @param sender Optional owner of the event. The owner is passed to
        * any listeners on the event being triggered along with any other
        * specified data.
        */
        EventBase(Sender sender = nullptr) :
            m_sender(sender)
        { /* Empty */ }

        /**
        * @brief Allows tranferring "ownership" of an event. Use with caution!
        *
        * @param sender Optional owner of the event. Setting nullptr disowns the event.
        */
        void set_sender(Sender sender) {
            m_sender = sender;
        }
    protected:
        Sender m_sender;
    };

    template <typename ...Parameters>
    using Subscriber  = Delegate<void(Sender, Parameters...)>;
    template <typename ...Parameters>
    using Subscribers = std::vector<Subscriber<Parameters...>*>;

    template <typename Functor, typename ...Parameters>
    concept CanSubscribe = requires(Functor f) {
        { new Subscriber<Parameters...>(std::forward<Functor>(f)) } -> std::same_as<Subscriber<Parameters...>*>;
    };

    /**
     * @brief The standard event class, provides the most trivial event system in
     * which subscribers are invoked in order of subscription and all subscribers
     * get processed.
     * 
     * @tparam Parameters Additional data passed to listeners when the event is
     * triggered.
     */
    template <typename ...Parameters>
    class Event : public EventBase {
    public:
        using _Subscriber = Subscriber<Parameters...>;
        using _Subscribers = Subscribers<Parameters...>;

        /**
        * @brief Default constructor
        *
        * @param sender Optional owner of the event. The owner is passed to
        * any listeners on the event being triggered along with any other
        * specified data.
        */
        Event(Sender sender = nullptr) :
            EventBase(sender),
            m_triggering(false)
        { /* Empty */ }
        /**
        * @brief Copies the event and subscribers.
        *
        * @param event The event to copy.
        */
        Event(const Event& event) :
            EventBase(event.m_sender) {
            m_subscribers   = event.m_subscribers;
            m_removal_queue = event.m_removal_queue;
            m_triggering    = false;
        }
        /**
        * @brief Moves the event. Ownership is guaranteed to be transferred.
        *
        * @param event The event object to move from.
        */
        Event(Event&& event) :
            EventBase(event.m_sender) {
            m_subscribers   = std::move(event.m_subscribers);
            m_removal_queue = std::move(event.m_removal_queue);
            m_triggering    = false;
        }

        /**
        * @brief Copies the event and subscribers.
        *
        * @param event The event to copy.
        *
        * @return The event that has been copied to.
        */
        Event& operator=(const Event& event) {
            m_sender        = event.m_sender;
            m_subscribers   = event.m_subscribers;
            m_removal_queue = event.m_removal_queue;
            m_triggering    = false;

            return *this;
        }
        /**
        * @brief Moves the event. Ownership is guaranteed to be transferred.
        *
        * @param event The event object to move from.
        *
        * @return The event that has been copied to.
        */
        Event& operator=(Event&& event) {
            m_sender        = event.m_sender;
            m_subscribers   = std::move(event.m_subscribers);
            m_removal_queue = std::move(event.m_removal_queue);
            m_triggering    = false;

            return *this;
        }

        /**
        * @brief Adds a subscriber to the event.
        *
        * @param subscriber The subscriber to add to the event.
        */
        void add(_Subscriber* subscriber) {
            m_subscribers.emplace_back(subscriber);
        }

        /**
        * @brief Adds a subscriber to the event.
        *
        * operator+= is an alias of add.
        *
        * @param subscriber The subscriber to add to the event.
        */
        void operator+=(_Subscriber* subscriber) {
            add(subscriber);
        }

        /**
        * @brief Adds a functor as a subscriber to this event.
        *
        * @tparam Functor The type of the functor to be subscribed.
        * @param functor The functor to be subscribed.
        * 
        * @warning This function uses new, don't call unless really needed.
        * 
        * @return The created subscriber object. Memory is the
        * responsibility of the caller.
        */
        template <typename Functor>
            requires CanSubscribe<Functor, Parameters...>
        _Subscriber* add_functor(Functor&& functor) {
            _Subscriber* sub = new _Subscriber(std::forward<Functor>(functor));

            add(sub);

            return sub;
        }

        /**
        * @brief Removes a subscriber from the event (all instances are removed).
        *
        * @param subscriber The subscriber to remove from the event.
        */
        void remove(_Subscriber* subscriber) {
            // We don't want to be invalidating iterators by removing subscribers mid-trigger.
            if (m_triggering) {
                m_removal_queue.emplace_back(subscriber);
            } else {
                std::erase_if(m_subscribers, [subscriber](_Subscriber* rhs) {
                    return rhs == subscriber;
                });
            }
        }
        /**
        * @brief Removes a subscriber from the event (all instances are removed).
        *
        * operator-= is an alias of remove
        *
        * @param subscriber The subscriber to remove from the event.
        */
        void operator-=(_Subscriber* subscriber) {
            remove(subscriber);
        }

        /**
        * @brief Triggers the event, invoking all subscribers.
        *
        * @param parameters The parameters to pass to subscribers.
        */
        void trigger(Parameters... parameters) {
            // We don't want to be invalidating iterators by removing subscribers mid-trigger.
            m_triggering = true;

            for (auto& subscriber : m_subscribers) {
                (*subscriber)(m_sender, parameters...);
            }

            m_triggering = false;

            // Remove any subscribers that requested to be unsubscribed during triggering.
            for (auto& unsubscriber : m_removal_queue) {
                std::erase_if(m_subscribers, [unsubscriber](_Subscriber* rhs) {
                    return rhs == unsubscriber;
                });
            }
        }
        /**
        * @brief Triggers the event, invoking all subscribers.
        *
        * operator() is an alias of trigger
        *
        * @param parameters The parameters to pass to subscribers.
        */
        void operator()(Parameters... parameters) {
            trigger(std::forward<Parameters>(parameters)...);
        }
    protected:
        _Subscribers m_subscribers;
        _Subscribers m_removal_queue;
        bool         m_triggering;
    };
}

#endif // __hemlock_event_h
