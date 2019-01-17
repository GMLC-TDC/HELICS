function v = helics_property_time_delta()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812681);
  end
  v = vInitialized;
end
