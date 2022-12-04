#ifndef __hemlock_event_h
#define __hemlock_event_h

namespace hemlock {
    using SmartSender = hmem::WeakHandle<const void*>;

    // Sender is the object responsible for handling the event, and is stored
    // simply as a void pointer; making things easier for us and harder for
    // users to do stupid things.
    class Sender {
    public:
        Sender() {
            m_is_smart  = false;
            m_ptr.smart = SmartSender();
        }

        ~Sender() {
            if (m_is_smart) m_ptr.smart.SmartSender::~weak_ptr();
        }

        Sender(std::nullptr_t) {
            m_is_smart   = false;
            m_ptr.simple = nullptr;
        }

        Sender(const Sender& rhs) {
            m_is_smart = rhs.m_is_smart;
            if (m_is_smart) {
                m_ptr.smart = rhs.m_ptr.smart;
            } else {
                m_ptr.simple = rhs.m_ptr.simple;
            }
        }

        Sender(Sender&& rhs) {
            m_is_smart = rhs.m_is_smart;
            if (m_is_smart) {
                m_ptr.smart = std::forward<SmartSender>(rhs.m_ptr.smart);
            } else {
                m_ptr.simple = rhs.m_ptr.simple;
            }
        }

        template <typename Type>
            requires std::is_pointer_v<Type>
        Sender(Type value) {
            m_is_smart   = false;
            m_ptr.simple = reinterpret_cast<const void*>(value);
        }

        template <typename Type>
            requires is_weak_ptr_v<Type>
        Sender(Type value) {
            m_is_smart  = true;
            m_ptr.smart = *reinterpret_cast<SmartSender*>(&value);
        }

        template <typename Type>
            requires is_shared_ptr_v<Type>
        Sender(Type value) {
            m_is_smart  = true;
            m_ptr.smart = SmartSender(reinterpret_pointer_cast<const void*>(value));
        }

        Sender& operator=(std::nullptr_t) {
            m_is_smart   = false;
            m_ptr.simple = nullptr;
            return *this;
        }

        Sender& operator=(const Sender& rhs) {
            m_is_smart = rhs.m_is_smart;
            if (m_is_smart) {
                m_ptr.smart = rhs.m_ptr.smart;
            } else {
                m_ptr.simple = rhs.m_ptr.simple;
            }
            return *this;
        }

        Sender& operator=(Sender&& rhs) {
            m_is_smart = rhs.m_is_smart;
            if (m_is_smart) {
                m_ptr.smart = std::forward<SmartSender>(rhs.m_ptr.smart);
            } else {
                m_ptr.simple = rhs.m_ptr.simple;
            }
            return *this;
        }

        template <typename Type>
        Type* get_ptr() {
            assert(!m_is_smart);

            return reinterpret_cast<Type*>(m_ptr.simple);
        }

        template <typename Type>
        hmem::WeakHandle<Type> get_handle() {
            assert(m_is_smart);

            return *reinterpret_cast<hmem::WeakHandle<Type>*>(&m_ptr.smart
            );  // reinterpret_pointer_cast<Type>(m_ptr.smart.lock());
        }
    protected:
        bool m_is_smart;

        union SimpleOrSmart {
            SimpleOrSmart() : smart(SmartSender()) { /* Empty. */
            }

            ~SimpleOrSmart() { /* Empty. */
            }

            const void* simple;
            SmartSender smart;
        } m_ptr;
    };

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
         * any subscribers on the event being triggered along with any other
         * specified data.
         */
        EventBase(Sender&& sender = nullptr) :
            m_sender(std::forward<Sender>(sender)) { /* Empty */
        }

        /**
         * @brief Allows tranferring "ownership" of an event. Use with caution!
         *
         * @param sender Optional owner of the event. Setting nullptr disowns the
         * event.
         */
        void set_sender(Sender&& sender) { m_sender = std::forward<Sender>(sender); }
    protected:
        Sender m_sender;
    };

    template <typename ReturnType>
    concept OrReturnable = requires (ReturnType r) {
                               {
                                   r |= r
                               };
                           };
    template <typename ReturnType>
    concept AccumulateReturnable = requires (ReturnType r) {
                                       {
                                           r.insert(
                                               r.end(),
                                               std::make_move_iterator(r.begin()),
                                               std::make_move_iterator(r.end())
                                           )
                                       };
                                   };
    template <typename ReturnType>
    concept Returnable = std::same_as<ReturnType, void> || OrReturnable<ReturnType>
                         || AccumulateReturnable<ReturnType>;

    template <Returnable ReturnType, typename... Parameters>
    using RSubscriber = Delegate<ReturnType(Sender, Parameters...)>;
    template <Returnable ReturnType, typename... Parameters>
    using RSubscribers = std::vector<RSubscriber<ReturnType, Parameters...>*>;

    template <typename... Parameters>
    using Subscriber = RSubscriber<void, Parameters...>;
    template <typename... Parameters>
    using Subscribers = RSubscribers<void, Parameters...>;

    template <typename Functor, typename ReturnType, typename... Parameters>
    concept CanRSubscribe
        = Returnable<ReturnType>
          && requires (Functor f) {
                 {
                     new RSubscriber<ReturnType, Parameters...>(
                         std::forward<Functor>(f)
                     )
                     } -> std::same_as<RSubscriber<ReturnType, Parameters...>*>;
             };

    template <typename Functor, typename... Parameters>
    concept CanSubscribe = CanRSubscribe<Functor, void, Parameters...>;

    /**
     * @brief The standard event class, provides the most trivial event system in
     * which subscribers are invoked in order of subscription and all subscribers
     * get processed.
     *
     * @tparam ReturnType The return type of subscriebrs of the event..
     * @tparam Parameters Additional data passed to subscribers when the event is
     * triggered.
     */
    template <Returnable ReturnType, typename... Parameters>
    class REvent : public EventBase {
    public:
        using _RSubscriber  = RSubscriber<ReturnType, Parameters...>;
        using _RSubscribers = RSubscribers<ReturnType, Parameters...>;

        /**
         * @brief Default constructor
         *
         * @param sender Optional owner of the event. The owner is passed to
         * any subscribers on the event being triggered along with any other
         * specified data.
         */
        REvent(Sender&& sender = nullptr) :
            EventBase(std::forward<Sender>(sender)), m_triggering(false) { /* Empty */
        }

        /**
         * @brief Copies the event and subscribers.
         *
         * @param event The event to copy.
         */
        REvent(const REvent& event) : EventBase(event.m_sender) {
            m_subscribers   = event.m_subscribers;
            m_removal_queue = event.m_removal_queue;
            m_triggering    = false;
        }

        /**
         * @brief Moves the event. Ownership is guaranteed to be transferred.
         *
         * @param event The event object to move from.
         */
        REvent(REvent&& event) : EventBase(std::move(event.m_sender)) {
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
        REvent& operator=(const REvent& event) {
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
        REvent& operator=(REvent&& event) {
            m_sender        = std::move(event.m_sender);
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
        void add(_RSubscriber* subscriber) { m_subscribers.emplace_back(subscriber); }

        /**
         * @brief Adds a subscriber to the event.
         *
         * operator+= is an alias of add.
         *
         * @param subscriber The subscriber to add to the event.
         */
        void operator+=(_RSubscriber* subscriber) { add(subscriber); }

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
        _RSubscriber* add_functor(Functor&& functor) {
            _RSubscriber* sub = new _RSubscriber(std::forward<Functor>(functor));

            add(sub);

            return sub;
        }

        /**
         * @brief Removes a subscriber from the event (all instances are removed).
         *
         * @param subscriber The subscriber to remove from the event.
         */
        void remove(_RSubscriber* subscriber) {
            // We don't want to be invalidating iterators by removing subscribers
            // mid-trigger.
            if (m_triggering) {
                m_removal_queue.emplace_back(subscriber);
            } else {
                std::erase_if(m_subscribers, [subscriber](_RSubscriber* rhs) {
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
        void operator-=(_RSubscriber* subscriber) { remove(subscriber); }

        /**
         * @brief Triggers the event, invoking all subscribers.
         *
         * @param parameters The parameters to pass to subscribers.
         */
        ReturnType trigger(Parameters... parameters) {
            if constexpr (OrReturnable<ReturnType>) {
                ReturnType result{};

                // We don't want to be invalidating iterators by removing subscribers
                // mid-trigger.
                m_triggering = true;
                for (auto& subscriber : m_subscribers) {
                    result |= (*subscriber)(m_sender, parameters...);
                }
                m_triggering = false;

                // Remove any subscribers that requested to be unsubscribed during
                // triggering.
                clear_removal_queue();

                return result;
            } else if constexpr (AccumulateReturnable<ReturnType>) {
                ReturnType result{};

                // We don't want to be invalidating iterators by removing subscribers
                // mid-trigger.
                m_triggering = true;
                for (auto& subscriber : m_subscribers) {
                    ReturnType partial_result
                        = (*subscriber)(m_sender, parameters...);
                    result.insert(
                        result.end(),
                        std::make_move_iterator(partial_result.begin()),
                        std::make_move_iterator(partial_result.end())
                    );
                }
                m_triggering = false;

                // Remove any subscribers that requested to be unsubscribed during
                // triggering.
                clear_removal_queue();

                return result;
            } else {
                // We don't want to be invalidating iterators by removing subscribers
                // mid-trigger.
                m_triggering = true;
                for (auto& subscriber : m_subscribers) {
                    (*subscriber)(m_sender, parameters...);
                }
                m_triggering = false;

                // Remove any subscribers that requested to be unsubscribed during
                // triggering.
                clear_removal_queue();
            }
        }

        /**
         * @brief Triggers the event, invoking all subscribers.
         *
         * operator() is an alias of trigger
         *
         * @param parameters The parameters to pass to subscribers.
         */
        ReturnType operator()(Parameters... parameters) {
            if constexpr (std::same_as<ReturnType, void>) {
                trigger(std::forward<Parameters>(parameters)...);
            } else {
                return trigger(std::forward<Parameters>(parameters)...);
            }
        }
    protected:
        void clear_removal_queue() {
            for (auto& unsubscriber : m_removal_queue) {
                std::erase_if(m_subscribers, [unsubscriber](_RSubscriber* rhs) {
                    return rhs == unsubscriber;
                });
            }
        }

        _RSubscribers m_subscribers;
        _RSubscribers m_removal_queue;
        bool          m_triggering;
    };

    template <typename... Parameters>
    using Event = REvent<void, Parameters...>;

    template <typename... Parameters>
    using CancellableEvent = REvent<bool, Parameters...>;
}  // namespace hemlock

#endif  // __hemlock_event_h
