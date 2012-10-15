
function event_loop()
   print("No more events pending");
end


function azra_process_events()
   
end

co = coroutine.create(event_loop);
coroutine.resume(co);
print(coroutine.status(co));

print("AZRA: entering main event loop"); 
