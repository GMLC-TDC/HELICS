function v = HELICS_CORE_TYPE_INTERPROCESS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107623);
  end
  v = vInitialized;
end
