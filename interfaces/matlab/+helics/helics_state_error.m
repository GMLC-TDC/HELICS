function v = helics_state_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856879);
  end
  v = vInitialized;
end
