function v = helics_time_property_rt_tolerance()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535406);
  end
  v = vInitialized;
end
