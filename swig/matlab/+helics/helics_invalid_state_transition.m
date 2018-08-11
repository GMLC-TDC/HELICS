function v = helics_invalid_state_transition()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 7);
  end
  v = vInitialized;
end
