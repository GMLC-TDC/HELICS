function v = helics_data_type_raw()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183053);
  end
  v = vInitialized;
end
