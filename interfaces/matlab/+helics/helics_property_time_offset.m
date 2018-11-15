function v = helics_property_time_offset()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183082);
  end
  v = vInitialized;
end
