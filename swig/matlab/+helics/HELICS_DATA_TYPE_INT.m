function v = HELICS_DATA_TYPE_INT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876595);
  end
  v = vInitialized;
end
