print("NULL transport URPC test");
print("Listing registered transports")
for i,j in pairs(__urpc_transports)do
   print(i,j)
end
print("Trying to create a null transport instance")
n = __urpc_open(__urpc_transports['nullt'], "test_instance")
print("result: ", n);
print("Trying discovery")
n = __urpc_discovery(n)
print("result: ", n);
print("Doing a test call")
n = __urpc_call(n)
print("result: ", n);
