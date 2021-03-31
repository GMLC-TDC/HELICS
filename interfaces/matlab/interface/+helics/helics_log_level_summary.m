function v = helics_log_level_summary()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 52);
  end
  v = vInitialized;
end
