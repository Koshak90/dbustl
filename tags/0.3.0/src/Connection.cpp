/*
 *  DBusTL - DBus Template Library
 *
 *  Copyright (C) 2008, 2009  Fabien Chevalier <chefabien@gmail.com>
 *  
 *
 *  This file is part of the DBus Template Library.
 *
 *  The DBus Template Library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  DBus Template Library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with DBus Template Library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <dbus/dbus.h>

#include <dbustl-1/Connection>
#include <dbustl-1/EventLoopIntegration>
#include <cassert>

namespace dbustl {

/** @cond */
//This is internal stuff, don't show it in doxygen
class ConnectionInitializer {
    ~ConnectionInitializer() {
        Connection::cleanup();  
    };
    static ConnectionInitializer _static;
};

ConnectionInitializer ConnectionInitializer::_static;
/** @endcond */

Connection* Connection::_system;
Connection* Connection::_session;
EventLoopIntegration* Connection::_defaultEventLoop;

void Connection::useEventLoop(const EventLoopIntegration& eventLoop)
{
    _defaultEventLoop = eventLoop.clone();
}

Connection* Connection::systemBus()
{
    if(!_system) {
        _system = new Connection(DBUS_BUS_SYSTEM);
    }
    return _system;
}

Connection* Connection::sessionBus()
{
    if(!_session) {
        _session = new Connection(DBUS_BUS_SESSION);
    }
    return _session;
}

DBusConnection* Connection::dbus()
{
    return _llconn;
}

Connection::Connection(DBusBusType bustype) : _eventLoop(0), _isPrivate(false)
{
    //As per DBUS documentation, it is safe to call it more than once, as
    //calls other than the first one are ignored
    dbus_threads_init_default();
    _llconn = dbus_bus_get_private(bustype, 0);

    if(_defaultEventLoop != 0) {
        _eventLoop = _defaultEventLoop->clone();
        _eventLoop->connect(this);
    }
}

Connection::Connection(DBusBusType bustype, const EventLoopIntegration& eventLoop) : 
  _eventLoop(eventLoop.clone()), _isPrivate(false)
{
    //As per DBUS documentation, it is safe to call it more than once, as
    //calls other than the first one are ignored
    dbus_threads_init_default();
    _llconn = dbus_bus_get_private(bustype, 0);
    
    _eventLoop->connect(this);
}

Connection::~Connection()
{
    delete _eventLoop;
    dbus_connection_close(_llconn);
    dbus_connection_unref(_llconn);
}

void Connection::cleanup() {
    delete _defaultEventLoop;
    _defaultEventLoop = 0;
    delete _system;
    _system = 0;
    delete _session;
    _session = 0;
    //Release all memory allocated internally by dbus library
    dbus_shutdown();
}

}
