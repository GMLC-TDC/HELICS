function v = helics_log_level_interfaces()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 54);
  end
  v = vInitialized;
end
