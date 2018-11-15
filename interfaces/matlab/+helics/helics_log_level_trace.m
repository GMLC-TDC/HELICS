function v = helics_log_level_trace()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183066);
  end
  v = vInitialized;
end
