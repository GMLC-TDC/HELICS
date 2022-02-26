function v = HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 98);
  end
  v = vInitialized;
end
