#pragma once
#include "helper.h"
#include <concepts>
#include <cstddef>
#include <functional>
#include <list>
#include <ranges>
#include <typeinfo>

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
class Leaf : public Node<T>
{
public:
    Leaf() : Node<T>()
    { this->_is_leaf = true; }
};

template <typename T>
class Node : public T
{
    Node<T>*            _parent;
    std::list<Node<T>*> _children;

protected:
    // object modifiers
    bool _is_root = false;
    bool _is_leaf = false;
public:
    DEFINE_POINTER_ITERATOR_WRAPPER(_children)

    inline bool empty() const noexcept
    { return _children.empty(); }

    inline size_t size() const noexcept
    { return _children.size(); }

    inline std::optref<Node<T>> front() const noexcept
    { return _children.front() ? std::optref<Node<T>>(*_children.front()) : std::nullopt; }

    inline std::optref<Node<T>> back() const noexcept
    { return _children.back() ? std::optref<Node<T>>(*_children.back()) : std::nullopt; }

    inline std::optref<Node<T>> parent()const noexcept
    { return (_parent) ? std::optref<Node<T>>(*_parent) : std::nullopt; }

    template <std::derived_from<Root<T>> R = Root<T>>
    inline const R& root() const
    { return static_cast<const R&>(get_root(*this)); }

    template <std::derived_from<Root<T>> R = Root<T>>
    inline R& root()
    { return static_cast<R&>(get_root(*this)); }

    template <typename Cast>
    inline bool convertible_to() const
    {
        if constexpr (std::derived_from<Cast, Leaf<T>>) {
            if (!_is_leaf) return false;
        } else if constexpr (std::derived_from<Cast, Root<T>>) {
            if (!_is_root) return false;
        } else {
            if (_is_root || _is_leaf) return false;
        }
        return true;
    }

    template <typename Cast>
    inline const Cast& get() const
    {
        if (!convertible_to<Cast>())
            throw std::bad_cast();
        return static_cast<const Cast&>(*this);
    }

    template <typename Cast>
    inline Cast& get()
    {
        if (!convertible_to<Cast>())
            throw std::bad_cast();
        return static_cast<Cast&>(*this);
    }

    inline bool is_leaf() const noexcept
    { return _is_leaf; }

    inline bool is_root() const noexcept
    { return _is_root; }

public:
    Node() noexcept = default;

    void add(Node<T>& node)
    {
        assert(!_is_leaf);
        assert(!node._is_root);
        assert(!(_is_root && node._is_leaf));
        node._parent = this;
        _children.push_back(&node);
    }

    void remove(Node<T>& node)
    {
        auto node_it = std::ranges::find(_children, &node);
        assert(node_it != _children.end());
        (*node_it)->_parent = nullptr;
        _children.erase(node_it);
    }

    void erase(const_iterator node_it)
    {
        node_it->_parent = nullptr;
        _children.erase(node_it);
    }

    void insert(const_iterator position, Node<T>& node)
    {
        assert(!_is_leaf);
        assert(!node._is_root);
        assert(!(_is_root && node._is_leaf));
        node._parent = this;
        _children.insert(position, &node);
    }

    void clear()
    { _children.clear(); }

    void shift_forward(const_iterator position)
    {
        _children.splice(std::ranges::next(position, 2), _children, position);
    }

    void shift_backward(const_iterator position)
    {
        _children.splice(std::ranges::prev(position), _children, position);
    }

    virtual ~Node() noexcept = default;
};

template <typename T>
inline const Root<T>& get_root(const Node<T>& node)
{
    const Node<T>* n = &node;
    while (n->parent()) n = &n->parent()->get();
    assert(n->is_root());
    return *n;
}

template <typename T>
inline Root<T>& get_root(Node<T>& node)
{
    Node<T>* n = &node;
    while (n->parent()) n = &n->parent()->get();
    assert(n->is_root());
    return static_cast<Root<T>&>(*n);
}

template <typename T>
inline Leaf<T>& get_front_leaf(Node<T>& node)
{
    Node<T>* leaf = &node;
    while (!leaf->is_leaf()) leaf = &leaf->front()->get();
    return static_cast<Leaf<T>&>(*leaf);
}

template <typename T>
inline Leaf<T>& get_back_leaf(Node<T>& node)
{
    Node<T>* leaf = &node;
    while (!leaf->is_leaf()) leaf = &leaf->back()->get();
    return static_cast<Leaf<T>&>(*leaf);
}

template <typename T>
inline void transfer(Node<T>& from, Node<T>& to)
{
    assert(from.parent());
    from.parent()->get().remove(from);
    to.add(from);
}
