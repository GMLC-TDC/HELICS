function v = helics_flag_rollback()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183062);
  end
  v = vInitialized;
end
