function v = helics_filter_type_clone()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812705);
  end
  v = vInitialized;
end
