function v = helics_property_time_delta()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183080);
  end
  v = vInitialized;
end
