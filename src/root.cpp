#include "root.h"
#include "atoms.h"
#include "config.h"
#include "logger.h"
#include "monitor.h"
#include "server.h"
#include "window.h"
#include <algorithm>

extern "C" {
#include <stdlib.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>
}

Root::Root(Server& srv) : srv(srv), rect(srv.rect())
{
    update_monitors();
}

Root& Root::instance(Server& srv)
{
    static Root root(srv);
    return root;
}

void Root::update_monitors()
{
    if (Config::randr_support) update_monitors_randr();
    else if (monitors.empty()) monitors.insert(new Monitor(srv, *this, rect));
    else (*monitors.begin())->rect = rect;
}

void Root::update_monitors_randr()
{
    xcb_generic_error_t* err;

    auto* monitors = xcb_randr_get_monitors_reply(srv(), xcb_randr_get_monitors(srv(), srv.root_window(), true), &err);
    if (err) {
        Log::error("Could not get RandR monitors: X11 err code {}", err->error_code);
        free(err);
        return;
    }

    // Tf is this complicated code
    // TODO: simplify to more readable and maintainable
    xcb_randr_monitor_info_iterator_t iter;
    for (iter = xcb_randr_get_monitors_monitors_iterator(monitors); iter.rem; xcb_randr_monitor_info_next(&iter)) {
        auto*       m_info = iter.data;
        std::string name   = srv.atom_name(m_info->name);
        Monitor*    mon    = get_monitor_by(name);
        if (!mon) {
            mon = new Monitor(srv, *this, rect);

            // Get output names
            auto outputs = xcb_randr_monitor_info_outputs(m_info);
            for (int i = 0; i < xcb_randr_monitor_info_outputs_length(m_info); i++) {
                auto* info = xcb_randr_get_output_info_reply(
                    srv(), xcb_randr_get_output_info(srv(), outputs[i], monitors->timestamp), NULL);
                std::string oname = (char*)xcb_randr_get_output_info_name(info);

                if (info && info->crtc != XCB_NONE)
                    if (name != oname) mon->names.push_back(oname);

                free(info);
                info = nullptr;
            }
        } else {
            mon->primary     = m_info->primary;
            mon->rect.x      = m_info->x;
            mon->rect.y      = m_info->y;
            mon->rect.width  = m_info->width;
            mon->rect.height = m_info->height;
        }
    }
    free(monitors);
}

bool Root::is_managed(xcb_window_t w)
{
    return managed_windows.contains(w);
}

Monitor* Root::get_monitor_by(const std::string& name) const
{
    for (const auto& m : monitors) {
        if (m->primary && name == "primary") return m;

        // If not primary, find name through monitor names
        for (const auto& n : m->names)
            if (n == name) return m;
    }
    return nullptr;
}

void Root::update_current_workspace()
{
    uint32_t id = focused->get_workspace()->index;
    srv.change_cardinal_property(srv.root_window(), _NET_CURRENT_DESKTOP, {&id, 1});
}

void Root::update_workspace_size()
{
    uint32_t size = workspaces.size();
    srv.change_cardinal_property(srv.root_window(), _NET_NUMBER_OF_DESKTOPS, {&size, 1});
}

void Root::update_workspace_names()
{
    std::vector<const char*> ws_names(workspaces.size());

    for (const auto& [id, w] : workspaces) ws_names.push_back(w->name.c_str());

    srv.change_string_property(srv.root_window(), _NET_DESKTOP_NAMES, ws_names);
}

void Root::update_workspace_viewport()
{
    std::vector<uint32_t> viewports(workspaces.size() * 2);

    for (const auto& [id, w] : workspaces) {
        viewports.push_back(w->rect.x);
        viewports.push_back(w->rect.y);
    }

    srv.change_cardinal_property(srv.root_window(), _NET_DESKTOP_VIEWPORT, viewports);
}

void Root::update_client_list()
{
    std::vector<xcb_window_t> window_list(managed_windows.size());

    for (const auto& w : managed_windows) window_list.push_back(w.first);

    srv.change_window_property(srv.root_window(), _NET_CLIENT_LIST, window_list);
}

void Root::update_active_window()
{
    xcb_window_t id = 0;
    if (focused->window) id = focused->window->id;

    srv.change_window_property(srv.root_window(), _NET_ACTIVE_WINDOW, {&id, 1});
}

Root::~Root()
{
    std::for_each(
        monitors.begin(), monitors.end(), [](const auto& mon) { delete mon; });
}
