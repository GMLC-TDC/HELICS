function v = helics_invalid_state_transition()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783843);
  end
  v = vInitialized;
end
