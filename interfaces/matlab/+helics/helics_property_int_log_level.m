function v = helics_property_int_log_level()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183089);
  end
  v = vInitialized;
end
