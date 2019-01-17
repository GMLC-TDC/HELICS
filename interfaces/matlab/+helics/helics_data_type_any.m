function v = helics_data_type_any()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812644);
  end
  v = vInitialized;
end
