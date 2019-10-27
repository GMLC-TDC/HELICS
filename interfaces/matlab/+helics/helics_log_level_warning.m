function v = helics_log_level_warning()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 40);
  end
  v = vInitialized;
end
