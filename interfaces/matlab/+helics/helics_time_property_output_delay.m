function v = helics_time_property_output_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107641);
  end
  v = vInitialized;
end
