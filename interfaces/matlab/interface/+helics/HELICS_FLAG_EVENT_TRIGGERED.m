function v = HELICS_FLAG_EVENT_TRIGGERED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 41);
  end
  v = vInitialized;
end
