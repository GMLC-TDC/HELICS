function v = helics_property_time_offset()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 61);
  end
  v = vInitialized;
end
