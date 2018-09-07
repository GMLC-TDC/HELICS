function v = helics_initialization_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876571);
  end
  v = vInitialized;
end
