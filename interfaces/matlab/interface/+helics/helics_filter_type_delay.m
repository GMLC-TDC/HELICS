function v = helics_filter_type_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 104);
  end
  v = vInitialized;
end
