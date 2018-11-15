function v = helics_log_level_timing()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183064);
  end
  v = vInitialized;
end
