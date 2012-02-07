-- This is a sample configuration file for azra
-- It is intended for interactive mode and is actually a lua 
-- script. 

-- Select the libraries to load
config.libraries = { "urpc" }
-- These will be loaded only when an interactive session is started
config.libraries_interactive = { "dumper", "help" }

-- define uRPC nodes 
config.urpc_nodes = {
{ "motor_cortex", "uart", "/dev/ttyUSB0:115200:8:n:1" },
{ "eyetower", "tcp", "eyetower", 8080 },
}

-- Connect at start
config.urpc_autoconnect=true
-- Do not stop, if nodes are not around
config.urpc_stop_on_error=false

-- Authentification function to start at login
config.auth="none"
-- Byte-compile libraries
config.bytecompile=true;
-- Load essential libraries, mostly for debugging
-- NOTE: These must be BELOW all the configuration statements
-- libload("dumper");
-- libload("help");
-- libload("urpc");
