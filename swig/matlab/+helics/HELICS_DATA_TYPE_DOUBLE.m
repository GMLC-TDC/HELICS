function v = HELICS_DATA_TYPE_DOUBLE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876594);
  end
  v = vInitialized;
end
