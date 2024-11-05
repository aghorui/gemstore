# Changes made to the libraries

* httplib: Removed the setting of `SO_REUSEPORT` from `default_socket_options`.
  This was removed because otherwise the server would share the port with
  another process, which we don't want (at least for now).