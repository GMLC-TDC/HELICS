function v = helics_data_type_int()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812636);
  end
  v = vInitialized;
end
