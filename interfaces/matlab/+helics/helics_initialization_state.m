function v = helics_initialization_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183053);
  end
  v = vInitialized;
end
