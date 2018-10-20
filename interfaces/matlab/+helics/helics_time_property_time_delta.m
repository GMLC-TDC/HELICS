function v = helics_time_property_time_delta()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535401);
  end
  v = vInitialized;
end
