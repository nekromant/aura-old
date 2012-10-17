print("NULL transport URPC test");
print("Listing registered transports")
for i,j in pairs(__urpc_transports)do
   print(i,j)
end

node = urpc_open("nil","sdfv")
node = urpc_open("null","totally-null-instance")
print("Hostname is: ", node.hostname())
node.packnumbers(1,2,3,4,5)
node.unpacknumbers()
