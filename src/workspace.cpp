#include "workspace.h"
#include "logger.h"
#include <algorithm>

void Focus_list::add(Focusable* foc, bool focus)
{
    current() ? current()->focused() ? current()->unfocus()
                                     : void(0) 
              : void(0);
    
    if (_pos.contains(foc)) {
        logger::debug("Adding exisiting focus");
        auto it = std::find(_list.begin(), _list.end(), foc);
        _list.splice(_list.end(), _list, it);
    } else {
        logger::debug("Adding new focus");
        _list.push_back(foc);
        _pos.insert(foc);
    }
    if (focus) foc->focus();
}

void Focus_list::remove(Focusable* foc)
{
    auto* last = current();
    if (foc->focused())
        foc->unfocus();
    logger::debug("Removing focus");
    _list.remove(foc);
    _pos.erase(foc);
    if (foc == last && current()) {
        logger::debug("Refocusing last focus");
        add(current());
    }
}

void Workspace::update_rect(const Vector2D& rect)
{
    logger::debug("Updating Workspace rect -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    Container::update_rect(rect);
    for (const auto& lcon : _children)
        lcon->update_rect(rect);
}
