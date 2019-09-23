function v = helics_property_int_file_log_level()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 70);
  end
  v = vInitialized;
end
