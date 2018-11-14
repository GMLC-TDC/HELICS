function v = helics_int_property_log_level()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230908);
  end
  v = vInitialized;
end
