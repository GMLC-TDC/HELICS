function v = helics_state_startup()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856875);
  end
  v = vInitialized;
end
