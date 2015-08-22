# About DBusTL #

DBusTL is an easy do to use, extensible C++ wrapper for D-Bus that makes the use of D-Bus with plain C++ easy.

For more help, including code source samples, see [the Online Documentation](http://dbustl.googlecode.com/svn/doc-latest.html).

Though already quite usefull, DBusTL is still in its infancy, and therefore interface might change at any time before 1.0 release.

**Important note**: To be used with the most elegant syntax, DBusTL requires a cutting edge C++ feature, that is currently only available with gcc 4.3 and upper. Use of older versions of gcc is possible, but will make the syntax a little more convoluted.

# Project status #

What's there:
  * Easy consumption of D-Bus services (client side D-Bus), provided you do not intend to receive signals.
  * Mapping of D-Bus types to standard STL containers.
  * Ability to receive signals when using Glib main loop.
  * Support representation of any struct or class as D-Bus struct.
  * Usable with or without C++ exceptions.

What's not there (yet):
  * Only glib main loop support is provided here.
  * Documentation of the way to integrate mainloop.
  * Server side support (exposing your own services on the bus).

# Roadmap #

FutureThoughts page lists how i'd like to see DBusTL move on from here. However due to the very limited spare time i have to work on this projet, i cannot make any commitment
to how fast thing will get in shape.

# News #

**June 6th, 2009** 0.4.0 is out. This is a bugfix/cleanup release:
  * Fixed building with gcc 4.4
  * Fixed deserialization of unsigned char type, that would abort.
  * Internal code refactoring and cleanup
  * Some minor API changes regarding custom structs and compat C++ mode
  * Support for (de)serialisation of STL containers with user-provided memory allocators
  * API incompatible change: Renamed ServerProxy as ObjectProxy

**April 7th, 2009** 0.3.0 is out. This release adds the following:
  * removes dependancy on dbus-glib for main loop support.
  * provides alternate DBusTL libraries built without C++ exceptions support.
  * more documentation has been written.

**March 24th, 2009** After 9 months of beeing kept prisoner on my own PC, i finally found time to push the beast on google code :-). So 0.2.0 is out the door, see [the 5 minutes guide to DBusTL](http://dbustl.googlecode.com/svn/tags/0.2.0/doc/html/intro.html).