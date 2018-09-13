function v = helics_state_execution()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183054);
  end
  v = vInitialized;
end
