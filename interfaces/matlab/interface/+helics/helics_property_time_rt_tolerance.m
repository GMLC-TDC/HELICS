function v = helics_property_time_rt_tolerance()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 73);
  end
  v = vInitialized;
end
