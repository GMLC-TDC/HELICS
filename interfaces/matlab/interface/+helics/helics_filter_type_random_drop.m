function v = helics_filter_type_random_drop()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 106);
  end
  v = vInitialized;
end
