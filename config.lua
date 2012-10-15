-- This is a sample configuration file for azra



config = { 
  -- Where to look for init files
  luadir = "./",
  -- And what init file tu use
  initfile = "lua/init.lua",
  -- initial set of libraries
  libraries={ "urpc" },
  -- load aditional libraries when interactive mode
  libraries_interactive={ "dumper", "help" },
  -- no auth
  auth="none",
  -- bytecompiles libraries when needed, speeds up loading
  bytecompile=true,
  -- uRPC settings
  urpc={
    autoconnect=true,
    nodes={
      { "motor_cortex", "uart", "/dev/ttyUSB0", 115200, 8, "n", 1 },
      { "eyetower", "tcp", "eyetower", 8080 } 
    },
    stop_on_error=false 
  } 
}

testv="test";