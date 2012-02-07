config = {}

print("azra: Starting environment"); 
-- Core functions that won't get reloaded at start

function echo(...)
  io.stdout:write(...);
end

function echon(...)
	io.stdout:write(....."\n\r");
end



function runfile(file)
  echon("Loading: "..file);
  dofile(file);
end

function libload(file)
  runfile("lua/lib/"..file..".lua");
end

-- Gets called each time a client connects
function hook_login()
	echon("Run help(); to get help. -- Cpt. Obvious");
end

function azra_reconf()
  config = {}
  runfile(azra_getconf());
  for i,v in ipairs(config.libraries) do
  libload(v);
  end
  echon("Reconfiguration complete");
end


function azra_gc()
  before = collectgarbage("count");
  collectgarbage();
  after = collectgarbage("count");
  echon("azra: Collecting garbage: "..before.."K -> "..after.."K");
  --TODO: Call C function to clean up things
end

function azra_memusage()
  after = collectgarbage("count");
  echon("azra: Memory usage: "..after.."K");
end

azra_reconf();

print("azra: environment ready"); 
