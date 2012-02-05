-- This is a sample configuration file for azra
-- It is intended for interactive mode and is actually a lua 
-- script. 


-- define uRPC nodes 
urpc_nodes = {
{ "motor_cortex", "uart", "/dev/ttyUSB0", 115200 },
{ "eyetower", "tcp", "eyetower", 8080 },
}

-- Connect at start
urpc_autoconnect=true;
-- Do not stop, if nodes are not around
urpc_stop_on_error=false; 

-- Load essential libraries, mostly for debugging
-- NOTE: These must be BELOW all the configuration statements
libload("dumper");
libload("help");
libload("urpc");
