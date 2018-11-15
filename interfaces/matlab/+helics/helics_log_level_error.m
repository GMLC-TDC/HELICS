function v = helics_log_level_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183059);
  end
  v = vInitialized;
end
