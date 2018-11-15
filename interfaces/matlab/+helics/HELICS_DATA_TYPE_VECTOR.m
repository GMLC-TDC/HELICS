function v = helics_data_type_vector()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183048);
  end
  v = vInitialized;
end
