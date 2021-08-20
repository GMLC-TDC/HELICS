function v = HELICS_PROPERTY_TIME_OFFSET()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 84);
  end
  v = vInitialized;
end
