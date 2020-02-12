function v = helics_data_type_complex()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 18);
  end
  v = vInitialized;
end
