function v = helics_int_property_log_level()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535410);
  end
  v = vInitialized;
end
