#pragma once

class Managed
{
public:
    using Managed_id = unsigned int;

private:
    Managed_id _id;

public:
    Managed(const Managed_id id) noexcept
        : _id(id)
    {}

    inline Managed_id index() const noexcept
    { return _id; }
};