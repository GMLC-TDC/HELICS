function v = helics_time_property_period()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856903);
  end
  v = vInitialized;
end
