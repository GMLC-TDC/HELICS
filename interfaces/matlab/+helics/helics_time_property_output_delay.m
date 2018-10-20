function v = helics_time_property_output_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535408);
  end
  v = vInitialized;
end
