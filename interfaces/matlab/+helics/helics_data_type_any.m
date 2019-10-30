function v = helics_data_type_any()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 24);
  end
  v = vInitialized;
end
