function v = helics_property_time_delta()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 64);
  end
  v = vInitialized;
end
