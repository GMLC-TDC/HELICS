function v = HELICS_FILTER_TYPE_RANDOM_DELAY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 111);
  end
  v = vInitialized;
end
