function v = helics_log_level_interfaces()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 49);
  end
  v = vInitialized;
end
