function v = HELICS_PROPERTY_TIME_DELTA()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 74);
  end
  v = vInitialized;
end
