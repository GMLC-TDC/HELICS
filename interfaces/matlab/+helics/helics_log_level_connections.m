function v = helics_log_level_connections()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183073);
  end
  v = vInitialized;
end
