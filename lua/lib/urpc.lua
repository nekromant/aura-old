
function urpc_connect(node)
  print("uRPC: Connecting node: "..node[1]);
  if (node[2] == "uart" ) then
      urpc_connect_via_uart(node[3],node[4]);
  elseif (node[2] == "tcp" ) then
      urpc_connect_via_tcp(node[3],node[4]);
  else
   error("uRPC transport '"..node[2].."' unknown");  
  end
end

function urpc_connect_via_uart(port,speed)
  print("uRPC: UART "..port.."@"..speed.."bps");
end

function urpc_connect_via_tcp(host,port)
  print("uRPC: TCP/IP "..host.."@"..port.."");
end


if (urpc_autoconnect) then
  print("Establishing connection to uRPC nodes");
  for k,v in pairs(urpc_nodes) do 
    urpc_connect(v);
  end
end
