function v = helics_filtertype_random_drop()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535386);
  end
  v = vInitialized;
end
