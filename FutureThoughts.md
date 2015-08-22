This is some kind of a TODO list i have in mind at the moment:

  * DONE: Don't depend on dbus-glib for main loop integration, by really implementing GlibEventLoop
  * IN PROGRESS: Support for server side DBUS (i.e. expose C++ objects as DBUS objects)
  * DONE: Test exceptions less support
  * Support DBus variant type: don't know if that's really used
  * Support for operating on a private bus.
  * Define and implement a policy for handling loss of DBus connection
  * Audit and document Thread safety