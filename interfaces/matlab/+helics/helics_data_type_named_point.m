function v = helics_data_type_named_point()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183050);
  end
  v = vInitialized;
end
