function v = HELICS_PROPERTY_TIME_RT_LAG()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 87);
  end
  v = vInitialized;
end
