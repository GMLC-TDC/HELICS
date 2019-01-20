function v = helics_error_invalid_state_transition()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812676);
  end
  v = vInitialized;
end
