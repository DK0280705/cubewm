#pragma once
#include "helper.h"
#include <concepts>
#include <functional>
#include <list>
#include <ranges>
#include <stdexcept>

class Node_exception : public std::runtime_error
{
public:
    Node_exception(const char* message) noexcept
        : std::runtime_error(message)
    {}
};

template <typename T>
class Node;

template <typename T>
class Root : public Node<T>
{
public:
    Root() : Node<T>()
    { this->_is_root = true; }
};

template <typename T>
class Node : public T
{
    Node<T>*            _parent;
    std::list<Node<T>*> _children;

protected:
    // object modifiers
    bool _is_root;

public:
    using Iterator       = typename std::list<Node<T>*>::iterator;
    using Const_iterator = typename std::list<Node<T>*>::const_iterator;

public:
    DECLARE_CONTAINER_WRAPPER(_children)

    inline std::optref<Node<T>> front() const noexcept
    { return _children.front() ? std::optref<Node<T>>(*_children.front()) : std::nullopt; }

    inline std::optref<Node<T>> back() const noexcept
    { return _children.back() ? std::optref<Node<T>>(*_children.back()) : std::nullopt; }

    inline std::optref<Node<T>> parent() const noexcept
    { return (_parent) ? std::optref<Node<T>>(*_parent) : std::nullopt; }

    template <std::derived_from<Root<T>> R = Root<T>>
    inline const R& root() const
    { return static_cast<const R&>(get_root(*this)); }

    template <std::derived_from<Root<T>> R = Root<T>>
    inline R& root()
    { return static_cast<R&>(get_root(*this)); }

    inline bool is_leaf() const noexcept
    { return empty(); }

    inline bool is_root() const noexcept
    { return _is_root; }

public:
    Node() noexcept = default;

    void add(Node<T>& node)
    {
        assert(!node._is_root);
        node._parent = this;
        _children.push_back(&node);
    }

    void remove(Node<T>& node)
    {
        assert(!node._is_root);
        node._parent = nullptr;
        _children.erase(std::ranges::find(_children, &node));
    }

    void insert(Const_iterator position, Node<T>& node)
    {
        assert(!node._is_root);
        node._parent = this;
        _children.insert(position, &node);
    }

    void shift(Const_iterator position, int shift_pos)
    {
        assert(shift_pos);
        auto it_pos = (shift_pos > 0) ? std::ranges::next(position, 1 + shift_pos)
                                      : std::ranges::prev(position, shift_pos);
        _children.splice(it_pos, _children, position);
    }

    virtual ~Node() noexcept = default;
};

template <typename T>
inline const Node<T>& get_root(const Node<T>& node)
{
    const Node<T>* n = &node;
    while (n->parent()) n = &n->parent()->get();
    if (!n->is_root()) throw Node_exception("Node is orphaned");
    return *n;
}

template <typename T>
inline Node<T>& get_root(Node<T>& node)
{
    Node<T>* n = &node;
    while (n->parent()) n = &n->parent()->get();
    if (!n->is_root()) throw Node_exception("Node is orphaned");
    return *n;
}

template <typename T>
inline void transfer(Node<T>& from, Node<T>& to)
{
    if (!from.parent()) throw Node_exception("Node is orphaned");
    from.parent()->get().remove(from);
    to.add(from);
}
