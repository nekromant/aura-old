function help()
  echon("	Azra survival guide");
  echon("azra_help(); - print a list of azra core functions exported to the environment");
  echon("azra_reconf(); - reread configuration file");
  echon("azra_reinit(); - reinit (soft) the environment");
  echon("azra_gc(); - perform garbage collection");
  echon("azra_memusage(); - show current memory usage");
end


function azra_help()
  echon("Azra core functions that have been exported to the environment");
  a = {azra_hooks()}
    for i,v in ipairs(a) do
      echon("* "..v);
    end
  echon("Total of "..#a.." functions");
end

function hhtest()
echon("WooHoo");
echon("WooHoo");
end
