function v = HELICS_DATA_TYPE_BOOLEAN()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107633);
  end
  v = vInitialized;
end
