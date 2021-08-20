function v = HELICS_PROPERTY_TIME_INPUT_DELAY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 88);
  end
  v = vInitialized;
end
