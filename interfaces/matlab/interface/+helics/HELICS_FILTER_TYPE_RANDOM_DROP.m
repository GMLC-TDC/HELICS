function v = HELICS_FILTER_TYPE_RANDOM_DROP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 112);
  end
  v = vInitialized;
end
