function v = helics_filtertype_random_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183098);
  end
  v = vInitialized;
end
