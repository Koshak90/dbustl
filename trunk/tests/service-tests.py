#!/usr/bin/env python

import sys

import dbus
bus = dbus.SessionBus()

xml1 = bus.get_object('com.example.SampleService', '/').Introspect()         
xml2 = bus.get_object('com.example.SampleService', '/ServerObject').Introspect()         
xml3 = bus.get_object('com.example.SampleService', '/ServerObject/Child').Introspect()         
xml4 = bus.get_object('com.example.SampleService', '/Not').Introspect()         
xml5 = bus.get_object('com.example.SampleService', '/Not/A').Introspect()         
xml6 = bus.get_object('com.example.SampleService', '/Not/A/Child').Introspect()         
xml7 = bus.get_object('com.example.SampleService', '/Not/A/Child/Class').Introspect()         

if len(sys.argv) > 1 and sys.argv[1] == '-w':
    f = open('introspection1.xml', 'w')
    f.write(xml1)
    f.close()
    f = open('introspection2.xml', 'w')
    f.write(xml2)
    f.close()
    f = open('introspection3.xml', 'w')
    f.write(xml3)
    f.close()
    f = open('introspection4.xml', 'w')
    f.write(xml4)
    f.close()
    f = open('introspection5.xml', 'w')
    f.write(xml5)
    f.close()
    f = open('introspection6.xml', 'w')
    f.write(xml6)
    f.close()
    f = open('introspection7.xml', 'w')
    f.write(xml7)
    f.close()
else:
    f = open('introspection1.xml')
    assert xml1 == f.read()
    f = open('introspection2.xml')
    assert xml2 == f.read()
    f = open('introspection3.xml')
    assert xml3 == f.read()
    f = open('introspection4.xml')
    assert xml4 == f.read()
    f = open('introspection5.xml')
    assert xml5 == f.read()
    f = open('introspection6.xml')
    assert xml6 == f.read()
    f = open('introspection7.xml')
    assert xml7 == f.read()

proxy = bus.get_object('com.example.SampleService',
                       '/ServerObject')
                       
proxy.test_callvoid0()
proxy.test_callvoid1(1)
proxy.test_callvoid2(1, 1)
proxy.test_callvoid3(1, 1, 1)
                       
assert proxy.test_call0() == 0
assert proxy.test_call1(1) == 1
assert proxy.test_call2(1, 1) == 2
assert proxy.test_call3(1, 1, 1) == 3

#We should complain if signature is invalid
try:
    proxy.test_call2(1, 0.5)
    assert False
except dbus.exceptions.DBusException, ex:
    pass

#We should complain if argument count does not match
try:
    proxy.test_call2(1, 1, 1)
    assert False
except dbus.exceptions.DBusException, ex:
    pass

#Exception test 1
try:
    proxy.test_ex1()
    assert False
except dbus.exceptions.DBusException, ex:
    assert str(ex) == "org.mycompany.test: This is a test exception"

#Exception test 2
try:
    proxy.test_ex2()
    assert False
except dbus.exceptions.DBusException, ex:
    assert str(ex) == "org.dbustl.CPPException: Error"

#Exception test 3
try:
    proxy.test_ex3()
    assert False
except dbus.exceptions.DBusException, ex:
    assert str(ex) == "org.dbustl.CPPException: Unknown C++ exception"

#Flexible executor test
assert proxy.test_flexible_executor(1.0, 1.0) == 1

#Flexible executor exception test
try:
    proxy.test_flexible_executor(1.0, 0.0)
    assert False
except dbus.exceptions.DBusException, ex:
    assert str(ex) == "org.mycompany.test: Division by 0"

#Test interfaces
proxy.test_if()
assert proxy.test_if(dbus_interface='com.example.Interface1') == 'com.example.Interface1'
assert proxy.test_if(dbus_interface='com.example.Interface2') == 'com.example.Interface2'

#FIXME: check those signals are really received
proxy.test_signal()
proxy.test_signal2()
proxy.test_signal3()

proxy.stop()

print "Ok"
