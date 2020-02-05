function v = helics_log_level_timing()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 47);
  end
  v = vInitialized;
end
