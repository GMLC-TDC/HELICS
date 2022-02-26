function v = HELICS_PROPERTY_INT_LOG_BUFFER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 99);
  end
  v = vInitialized;
end
