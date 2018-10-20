function v = helics_time_property_period()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535402);
  end
  v = vInitialized;
end
