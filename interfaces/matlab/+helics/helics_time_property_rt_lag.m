function v = helics_time_property_rt_lag()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183080);
  end
  v = vInitialized;
end
