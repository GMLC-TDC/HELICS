function v = HELICS_LOG_LEVEL_NO_PRINT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 55);
  end
  v = vInitialized;
end
