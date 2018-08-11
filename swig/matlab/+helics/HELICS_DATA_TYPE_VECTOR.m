function v = HELICS_DATA_TYPE_VECTOR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 44);
  end
  v = vInitialized;
end
