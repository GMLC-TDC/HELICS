function v = helics_time_property_rt_tolerance()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230904);
  end
  v = vInitialized;
end
