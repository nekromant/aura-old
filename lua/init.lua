require "lfs"

print("azra: Warming up lua environment"); 

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

-- Library byte-compiling routines  
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

-- Gets called each time a client connects
function hook_login()
echon("Run help(); to get help. -- Cpt. Obvious");
end

function load_plugin(name)
   for i,j in pairs(config.pluginpaths) do
      if nil ~= do_azra_load_plugin(j.."lib"..name..".so") then
	 return true
      end      
   end
   print("Failed to load plugin: "..name);
end

runfile(configfile)

-- Now, let's load plugins, if any
print("Loading plugins")
for i,j in pairs(config.plugins) do
   load_plugin(j)
end


function urpc_open(name, ...)
   local node = { }
   tr = __urpc_transports[name]
   if nil == tr then
      print("fatal: transport is not avaliable")
      return nil;
   end
   node.__transport = tr;
   node.__instance = __urpc_open(tr, ...);
   if nil == node.__instance then
      print("fatal: call to open the instance failed");
      return nil
   end
   n = __urpc_discovery(node.__instance)
   for i,j in pairs(n) do
      if j['is_method'] then
	 node[j['name']] = function(...) 
	    return __urpc_call(node.__instance, i-1, ...)
	 end
      end
      if j['is_event'] then
	 node[j['name']] = function(...)
	    print("Event "..j['name'].." occured")
	    print(unpack(arg))
	 end
      end
   end
   return node
end


print("azra: environment ready"); 
