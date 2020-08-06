function v = helics_property_time_output_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 77);
  end
  v = vInitialized;
end
