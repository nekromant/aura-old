
require "lfs"
  
function check_lib(path,lib)
	local lua = path..'/'..lib..".lua"
	local luac = path..'/'..lib..".luac"
	local attr = lfs.attributes (lua)
	local attrc = lfs.attributes (luac)
	assert (type(attr) == "table")
	if type(attrc) ~= "table" or attr.change>attrc.change then
		echon("compiling library "..path.."/"..lib.."...");	
 		return os.execute("luac -o "..path.."/"..lib..".luac "..path.."/"..lib..".lua")
		end
	return 0
end

config = {}

print("azra: Starting environment"); 
-- Core functions that won't get reloaded at start

function echo(...)
  io.stdout:write(...);
end

function echon(...)
	echo(....."\n");
end


function runfile(file)
  echon("loading: "..file);
  dofile(file);
end

function libload(file)
	if (config.bytecompile) then
		local r = check_lib("lua/lib",file)
		if (0~=r) then
			echon("byte-compile failed, trying regular runfile");
			runfile("lua/lib/"..file..".lua")
		else
		runfile("lua/lib/"..file..".luac")
		end
	else
		runfile("lua/lib/"..file..".lua")	
	end
end

-- Gets called each time a client connects
function hook_login()
echon("Run help(); to get help. -- Cpt. Obvious");
end
-- Gets called when first client connects
function hook_first_login()
	libload_batch(config.libraries_interactive);
end

function libload_batch(libs)
	for i,v in ipairs(libs) do
	libload(v);
	end
end


function azra_reconf()
  config = {}
  config.urpc = {}
  runfile(azra_getconf());
  echon("configuration complete");
end


function azra_gc()
  before = collectgarbage("count");
  collectgarbage();
  after = collectgarbage("count");
  echon("Collecting garbage: "..before.."K -> "..after.."K");
  --TODO: Call C function to clean up things
end

function azra_memusage()
  after = collectgarbage("count");
  echon("Memory usage: "..after.."K");
end

azra_reconf();
libload_batch(config.libraries);
print("environment ready"); 
