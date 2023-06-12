#pragma once

namespace X11 {
class Connection;
namespace extension {

struct Extension
{
    int  base_event{};
    bool is_supported{};
};

struct XKB_extension : public Extension
{};

struct Xrandr_extension : public Extension
{
    bool have_randr_13{};
    bool have_randr_15{};
};

struct Xshape_extension : public Extension
{};

auto xkb()    noexcept -> const XKB_extension&;
auto xrandr() noexcept -> const Xrandr_extension&;
auto xshape() noexcept -> const Xshape_extension&;

void init(const Connection& conn);

} // namespace extension
} // namespace X11
