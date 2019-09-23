function v = helics_filter_type_clone()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 87);
  end
  v = vInitialized;
end
