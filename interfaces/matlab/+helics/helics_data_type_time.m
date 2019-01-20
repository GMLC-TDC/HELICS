function v = helics_data_type_time()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812642);
  end
  v = vInitialized;
end
