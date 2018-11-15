function v = helics_log_level_timing()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183075);
  end
  v = vInitialized;
end
