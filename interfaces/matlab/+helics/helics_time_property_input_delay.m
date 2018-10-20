function v = helics_time_property_input_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535407);
  end
  v = vInitialized;
end
