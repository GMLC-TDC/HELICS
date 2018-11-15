function v = helics_property_time_period()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183081);
  end
  v = vInitialized;
end
