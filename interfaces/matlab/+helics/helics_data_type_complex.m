function v = helics_data_type_complex()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812637);
  end
  v = vInitialized;
end
