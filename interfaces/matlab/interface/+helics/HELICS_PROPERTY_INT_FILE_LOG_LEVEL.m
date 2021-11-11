function v = HELICS_PROPERTY_INT_FILE_LOG_LEVEL()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 93);
  end
  v = vInitialized;
end
