function v = helics_property_time_rt_lead()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 84);
  end
  v = vInitialized;
end
