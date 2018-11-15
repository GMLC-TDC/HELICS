function v = helics_state_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183125);
  end
  v = vInitialized;
end
