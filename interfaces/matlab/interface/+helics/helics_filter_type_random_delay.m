function v = helics_filter_type_random_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 91);
  end
  v = vInitialized;
end
