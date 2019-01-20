function v = helics_data_type_double()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812635);
  end
  v = vInitialized;
end
