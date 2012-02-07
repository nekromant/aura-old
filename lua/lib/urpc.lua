
urpc={}
urpc.online = { "alpha", "beta" }
urpc.offline = { "gamma" }


function urpc_online()
	for i,k in ipairs(urpc.online) do
	echon("["..i.."] "..k);
	end
 	echon(count(urpc.online).." uRPC nodes currently online");
end

function urpc_offline()
	for i,k in ipairs(urpc.offline) do
	echon("["..i.."] "..k);
	end
	echon(urpc.online.." uRPC nodes currently online");
end

function urpc_connect(node)
  echon("uRPC: Connecting node: "..node[1]);
  if (node[2] == "uart" ) then
      urpc_connect_via_uart(node[3],node[4]);
  elseif (node[2] == "tcp" ) then
      urpc_connect_via_tcp(node[3],node[4]);
  else
   echon("uRPC transport '"..node[2].."' unknown");  
  end
end

function urpc_connect_via_uart(port,speed)
  echon("uRPC: UART "..port.."@"..speed.."bps");
end

function urpc_connect_via_tcp(host,port)
  echon("uRPC: TCP/IP "..host.."@"..port.."");
end


if (urpc_autoconnect) then
  echon("Establishing connection to uRPC nodes");
  for k,v in pairs(urpc_nodes) do 
    urpc_connect(v);
  end
end
