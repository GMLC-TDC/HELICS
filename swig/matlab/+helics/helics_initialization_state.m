function v = helics_initialization_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176321);
  end
  v = vInitialized;
end
