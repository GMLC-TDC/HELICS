function v = helics_data_type_named_point()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 20);
  end
  v = vInitialized;
end
