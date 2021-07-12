function v = HELICS_ERROR_INVALID_STATE_TRANSITION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 65);
  end
  v = vInitialized;
end
