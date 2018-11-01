function v = helics_time_property_rt_lag()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095538);
  end
  v = vInitialized;
end
