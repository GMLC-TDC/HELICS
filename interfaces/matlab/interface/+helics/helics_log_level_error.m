function v = helics_log_level_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 56);
  end
  v = vInitialized;
end
