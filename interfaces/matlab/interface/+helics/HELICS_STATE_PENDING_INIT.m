function v = HELICS_STATE_PENDING_INIT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 141);
  end
  v = vInitialized;
end
