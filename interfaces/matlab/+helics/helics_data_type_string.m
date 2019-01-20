function v = helics_data_type_string()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812634);
  end
  v = vInitialized;
end
