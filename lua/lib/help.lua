function help()
  print("azra_help - print a list of azra functions exported to the environment");
  print("reconf - reread configuration file");
end


function azra_help()
  print("Azra core functions that have been exported to the environment");
  a = {azra_hooks()}
    for i,v in ipairs(a) do
      print(v);
    end
  print("Total of ", #a, "functions");
end

print("Run help(); to get help. -- Cpt. Obvious");
