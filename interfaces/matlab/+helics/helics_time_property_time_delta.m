function v = helics_time_property_time_delta()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856902);
  end
  v = vInitialized;
end
