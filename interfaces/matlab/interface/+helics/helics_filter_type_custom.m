function v = helics_filter_type_custom()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 108);
  end
  v = vInitialized;
end
