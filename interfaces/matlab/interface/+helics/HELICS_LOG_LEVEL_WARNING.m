function v = HELICS_LOG_LEVEL_WARNING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 51);
  end
  v = vInitialized;
end
