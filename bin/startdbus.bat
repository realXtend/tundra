
:: session bus address
set DBUS_SESSION_BUS_ADDRESS=tcp:host=localhost,port=12434

:: system bus address
set DBUS_SYSTEM_BUS_DEFAULT_ADDRESS=tcp:host=localhost,port=12434 

:: Launch dbus daemon application
dbus\dbus-daemon.exe --config-file=data\session.conf
