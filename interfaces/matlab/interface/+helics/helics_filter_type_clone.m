function v = helics_filter_type_clone()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 110);
  end
  v = vInitialized;
end
