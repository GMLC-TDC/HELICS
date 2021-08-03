function v = HELICS_STATE_PENDING_TIME()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 137);
  end
  v = vInitialized;
end
