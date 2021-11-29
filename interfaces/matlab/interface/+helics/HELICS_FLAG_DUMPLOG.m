function v = HELICS_FLAG_DUMPLOG()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 54);
  end
  v = vInitialized;
end
