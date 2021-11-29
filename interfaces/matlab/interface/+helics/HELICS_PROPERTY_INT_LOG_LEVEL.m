function v = HELICS_PROPERTY_INT_LOG_LEVEL()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 94);
  end
  v = vInitialized;
end
