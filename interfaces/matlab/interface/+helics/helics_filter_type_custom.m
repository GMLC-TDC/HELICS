function v = helics_filter_type_custom()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 89);
  end
  v = vInitialized;
end
