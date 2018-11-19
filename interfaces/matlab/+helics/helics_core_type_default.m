function v = helics_core_type_default()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183032);
  end
  v = vInitialized;
end
