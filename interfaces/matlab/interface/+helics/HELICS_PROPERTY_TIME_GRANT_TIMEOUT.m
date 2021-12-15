function v = HELICS_PROPERTY_TIME_GRANT_TIMEOUT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 93);
  end
  v = vInitialized;
end
