function v = helics_error_invalid_state_transition()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183075);
  end
  v = vInitialized;
end
