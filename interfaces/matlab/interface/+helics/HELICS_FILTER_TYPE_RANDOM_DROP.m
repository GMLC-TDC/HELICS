function v = HELICS_FILTER_TYPE_RANDOM_DROP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 126);
  end
  v = vInitialized;
end
