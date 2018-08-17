function v = HELICS_DATA_TYPE_BOOLEAN()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783882);
  end
  v = vInitialized;
end
