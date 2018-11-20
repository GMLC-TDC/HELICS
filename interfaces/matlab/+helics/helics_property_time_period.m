function v = helics_property_time_period()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183092);
  end
  v = vInitialized;
end
