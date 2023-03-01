#pragma once

template <typename Type, typename...Types>
struct Visitable : public Visitable<Type>, public Visitable<Types...>
{
    using Visitable<Type>::accept;
    using Visitable<Types...>::accept;
};

template <typename Type>
struct Visitable<Type>
{
    virtual void accept(Type&& visitor) = 0;
};

template <typename Derived>
struct Visitable_static
{
    template <typename Visitor>
    requires requires(Visitor&& v, Derived&& d)
    { v(d); }
    void accept(Visitor&& visitor)
    { visitor(*static_cast<Derived*>(this)); }
};

#define VISITABLE_OVERRIDE(Type) \
    public: void accept(Type&& visitor) override { visitor(*this); }

#define VISITABLE_OVERRIDE_BASE(Type) \
    public: virtual void accept(Type&& visitor) override { throw std::runtime_error("Called base Visitable"); }
