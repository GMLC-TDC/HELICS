function v = helics_time_property_rt_lead()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535405);
  end
  v = vInitialized;
end
