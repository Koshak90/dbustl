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
#include <dbustl-1/ServerProxy>

#include <sstream>
#include <cassert>

namespace dbustl {

DBusObjectPathVTable ServerProxy::_vtable = {
    NULL,
    &ServerProxy::signalsProcessingMethod, 
    0,
    0,
    0,
    0
};

ServerProxy::ServerProxy(Connection* conn, const std::string& path, const std::string& destination) :
  _conn(conn), _path(path), _destination(destination), _timeout(-1)
{
    assert(_conn->isConnected());
    if(!dbus_connection_register_object_path(_conn->dbus(), _path.c_str(), &_vtable, this)) {
        throw_or_set(DBUS_ERROR_NO_MEMORY, "Not enough memory to register object path handler");
    }
}

ServerProxy::~ServerProxy()
{
    dbus_connection_unregister_object_path(_conn->dbus(), _path.c_str());

    std::map<std::string, FunctorWrapper>::iterator it;
    while((it = _signalsHandlers.begin()) != _signalsHandlers.end()) {
        // Beware : due to the fact the method below erases() the map element while we still
        // need some bits of its (i.e. signal name), we have to make a copy
        std::string sigName = it->first;
        //C++ Destructors must not throw, so protect those calls
        try {
            removeSignalHandler(sigName);
        }
        catch(const DBusException&) {
            //Skip exception
        }
    }
}

Message ServerProxy::createMethodCall(const std::string& methodName)
{
    const char *dest = _destination.empty() ? NULL : _destination.c_str();
    const char *intf = _interface.empty() ? NULL : _interface.c_str();
    
    Message method_call(dbus_message_new_method_call(dest, _path.c_str(), 
                    intf, methodName.c_str()));
                    
    if(method_call.dbus()) {
        // Blank out error status
        errorReset();
    }
    else {
        throw_or_set(DBUS_ERROR_NO_MEMORY, "Not enough memory to allocate DBUS message");
    }

    return method_call;
}

Message ServerProxy::call(Message& method_call)
{
    DBusException error;
    DBusMessage *reply;
          
    reply = dbus_connection_send_with_reply_and_block(_conn->dbus(), method_call.dbus(), _timeout, error.dbus());
    if(!reply) {
        throw_or_set(error);
    }
    return Message(reply);
}

void ServerProxy::processInArgs(Message& msg)
{
    if(!msg.error()) {
        Message reply(call(msg));
        // Overwrite message 
        msg = reply;
    }
}

void ServerProxy::processOutArgs(Message& reply)
{
    if(reply.error()) {
        throw_or_set( *(reply.error()) );
    }
}

void ServerProxy::executeAsyncCall(Message& method_call, DBusPendingCallNotifyFunction function,
    void* user_data, DBusFreeFunction free_user_data)
{
    DBusPendingCall *pending_return;

    if(dbus_connection_send_with_reply(_conn->dbus(), method_call.dbus(), &pending_return, _timeout) == FALSE) {
        throw_or_set(DBUS_ERROR_NO_MEMORY, "Not enough memory to send D-Bus message");
        return;
    }

    if(pending_return) {
        if(dbus_pending_call_set_notify(pending_return, function, 
              user_data, free_user_data) == FALSE) {
            throw_or_set(DBUS_ERROR_NO_MEMORY, "Not enough memory to set callback for D-Bus message");
        }
    }
    else {
        //we borrowed this one from dbus library, to be in sync with what call() would do.
        throw_or_set(DBUS_ERROR_DISCONNECTED, "Connection is closed");
    }
}

DBusHandlerResult ServerProxy::signalsProcessingMethod(DBusConnection *, 
    DBusMessage *dbusMessage, void *user_data)
{
    ServerProxy* proxy = static_cast<ServerProxy *>(user_data);
    
    /* Basic sanity check : libdbus tries an approximate match on objects
     * paths, but we want a perfect match */
    
    if(proxy->_path != dbus_message_get_path(dbusMessage)) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    
    Message msg(dbusMessage);

    /* libdbus keeps ownership of the message, but our Message class
     * wants ownership too: as a result both will free the message
     * once it is not used anymore. Ref it one more time as a workaround. */
    dbus_message_ref(msg.dbus());

    if(dbus_message_get_type(msg.dbus()) == DBUS_MESSAGE_TYPE_SIGNAL) {
        std::string sigName = msg.member();
        std::string handlerName;
        
        //First check if we can have an exact match
        if(proxy->_signalsHandlers.count(sigName)) {
            handlerName = sigName;
        }
        //Second check for a generic handler
        else if(proxy->_signalsHandlers.count("")) {
            handlerName = "";
        }
        else {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }
        
        FunctorWrapper& fw = proxy->_signalsHandlers[handlerName];
        
        (*fw.call_functor)(fw.functor, msg);

    	return DBUS_HANDLER_RESULT_HANDLED;
    }
    else {
    	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
}

void ServerProxy::removeSignalHandler(const std::string& signalName)
{
    if(_signalsHandlers.count(signalName)) {
        _signalsHandlers[signalName].delete_functor(_signalsHandlers[signalName].functor);
        _signalsHandlers.erase(signalName);
        setWatchSignal(signalName, false);
    }
}

void ServerProxy::setWatchSignal(const std::string& signalName, bool enable)
{
    if(!_conn->isPrivate()) {
        std::stringstream match;
        DBusException error;
        
        //Reset global error status
        errorReset();

        match << "type='signal',path='" << _path << "'";
        
        if(signalName.size()) {
            match << ",member='" << signalName << "'";
        }
        
        if(enable) {
            dbus_bus_add_match(_conn->dbus(), match.str().c_str(), error.dbus());
        }
        else {
            dbus_bus_remove_match(_conn->dbus(), match.str().c_str(), error.dbus());
        }
        
        if(error.isSet()) {
            throw_or_set(error);
        }
    }
}

}
