function v = HELICS_DATA_TYPE_MULTI()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 27);
  end
  v = vInitialized;
end
