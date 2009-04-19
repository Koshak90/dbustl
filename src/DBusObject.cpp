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

#include <dbustl-1/DBusObject>
#include <dbustl-1/Connection>
#include <dbustl-1/Message>

#include <cassert>

namespace dbustl {

DBusObjectPathVTable DBusObject::_vtable = {
    NULL,
    &DBusObject::incomingMessagesProcessing, 
    0,
    0,
    0,
    0
};

DBusObject::DBusObject(const std::string& objectPath, Connection *conn) : _conn(0), _objectPath(objectPath)
{
    if(conn) {
        enable(conn);
    }
}
            
void DBusObject::enable(Connection * conn)
{
    assert(conn->isConnected());
    errorReset();
    if(_conn) {
        disable();
    }

    DBusException ex;
    if(dbus_connection_try_register_object_path(conn->dbus(), _objectPath.c_str(), &_vtable, this, ex.dbus())) {
        _conn = conn;
    }
    else {
        throw_or_set(ex);
    }
}

void DBusObject::disable()
{
    errorReset();
    dbus_connection_unregister_object_path(_conn->dbus(), _objectPath.c_str());
    _conn = 0;
}

void DBusObject::exportMethod(const std::string& methodName, MethodExecutorBase *executor)
{
    if(_exportedMethods.count(methodName)) {
        delete _exportedMethods[methodName];
    }
    _exportedMethods[methodName] = executor;
}

DBusHandlerResult DBusObject::incomingMessagesProcessing(DBusConnection *, 
    DBusMessage *dbusMessage, void *user_data)
{   
    if(dbus_message_get_type(dbusMessage) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
        /* libdbus keeps ownership of the message, but our Message class
         * wants ownership too: as a result both will free the message
         * once it is not used anymore. Ref it one more time as a workaround. */
        dbus_message_ref(dbusMessage);
        
        Message call(dbusMessage);
        std::string methodName = call.member();
        DBusObject* object = static_cast<DBusObject *>(user_data);

        if(object->_exportedMethods.count(methodName)) {
            Message reply(dbus_message_new_method_return(call.dbus()));
            if(reply.dbus()) {
                try {
                    object->_exportedMethods[methodName]->processCall(&call, &reply);
                    dbus_connection_send(object->_conn->dbus(), reply.dbus(), NULL);
                }
            #ifndef DBUSTL_NO_EXCEPTIONS
                catch(const DBusException& e) {
                    Message errorReply(dbus_message_new_error(call.dbus(), e.name().c_str(), e.message().c_str()));
                    if(errorReply.dbus()) {
                        dbus_connection_send(object->_conn->dbus(), errorReply.dbus(), NULL);
                    }
                }
                catch(const std::exception& e) {
                    Message errorReply(dbus_message_new_error(call.dbus(), "org.dbustl.CPPException", e.what()));
                    if(errorReply.dbus()) {
                        dbus_connection_send(object->_conn->dbus(), errorReply.dbus(), NULL);
                    }
                }
                catch(...) {
                    Message errorReply(dbus_message_new_error(call.dbus(), "org.dbustl.CPPException", "Unknown C++ exception"));
                    if(errorReply.dbus()) {
                        dbus_connection_send(object->_conn->dbus(), errorReply.dbus(), NULL);
                    }
                }
            #endif
            }
      	    return DBUS_HANDLER_RESULT_HANDLED;
        }
    }
  	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

}
