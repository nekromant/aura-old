print("*** serial transport uRPC test ***");
print("Listing registered transports")
for i,j in pairs(__urpc_transports)do
   print(i,j)
end


s = urpc_open("serial","/dev/ttyUSB0:115200:8:n:1");
