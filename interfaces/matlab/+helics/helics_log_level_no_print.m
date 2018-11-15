function v = helics_log_level_no_print()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183058);
  end
  v = vInitialized;
end
