function v = HELICS_LOG_LEVEL_DATA()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 63);
  end
  v = vInitialized;
end
