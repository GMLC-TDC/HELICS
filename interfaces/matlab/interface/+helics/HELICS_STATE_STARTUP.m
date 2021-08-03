function v = HELICS_STATE_STARTUP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 130);
  end
  v = vInitialized;
end
