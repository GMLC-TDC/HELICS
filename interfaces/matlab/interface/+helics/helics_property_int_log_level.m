function v = helics_property_int_log_level()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 76);
  end
  v = vInitialized;
end
