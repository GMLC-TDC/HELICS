function v = helics_property_time_rt_lag()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183094);
  end
  v = vInitialized;
end
