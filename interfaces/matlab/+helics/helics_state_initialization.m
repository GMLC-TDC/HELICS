function v = helics_state_initialization()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856876);
  end
  v = vInitialized;
end
