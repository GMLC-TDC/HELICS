function v = HELICS_LOG_LEVEL_CONNECTIONS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 53);
  end
  v = vInitialized;
end
