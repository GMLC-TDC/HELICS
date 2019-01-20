function v = helics_data_type_vector()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812638);
  end
  v = vInitialized;
end
