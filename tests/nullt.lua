print("NULL transport URPC test");
print("Listing registered transports")
for i,j in pairs(__urpc_transports)do
   print(i,j)
end
print("Trying to create a null transport instance")
inst = __urpc_open(__urpc_transports['nullt'], "test_instance")
print("result: ", inst);
print("Trying discovery and dumping stuff I discovered")
n = __urpc_discovery(inst)
for i,j in pairs(n) do
   if j['is_method'] then 
      type = "method"
   else
      type = "event"
   end
   print("\n"..type.."\t#"..i)
   print("name:\t"..j['name']);
   if j['args'] then print("args:\t"..j['args']) end
   if j['reply'] then print("reply:\t"..j['reply']) end
   print("")
end
print("result: ", n);
print("Doing a test call")
n = __urpc_call(inst, 1)
print("result: ", n);
