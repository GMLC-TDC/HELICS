function v = helics_flag_source_only()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183047);
  end
  v = vInitialized;
end
