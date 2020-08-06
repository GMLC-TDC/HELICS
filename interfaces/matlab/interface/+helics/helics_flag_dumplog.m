function v = helics_flag_dumplog()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 45);
  end
  v = vInitialized;
end
