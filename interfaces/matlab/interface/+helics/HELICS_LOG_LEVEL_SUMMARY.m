function v = HELICS_LOG_LEVEL_SUMMARY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 60);
  end
  v = vInitialized;
end
