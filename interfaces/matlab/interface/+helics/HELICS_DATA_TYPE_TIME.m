function v = HELICS_DATA_TYPE_TIME()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 25);
  end
  v = vInitialized;
end
