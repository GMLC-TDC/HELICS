function v = HELICS_FILTER_TYPE_DELAY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 110);
  end
  v = vInitialized;
end
