function v = helics_startup_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876570);
  end
  v = vInitialized;
end
