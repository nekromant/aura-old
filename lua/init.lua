print("azra: Starting environment"); 
-- Core functions that won't get reloaded at start

function runfile(file)
  print("Loading: ",file);
  dofile(file);
end

function libload(file)
  runfile("lua/lib/"..file..".lua");
end



function azra_reconf()
  runfile(azra_getconf());
end

azra_reconf();

print("azra: environment ready"); 
