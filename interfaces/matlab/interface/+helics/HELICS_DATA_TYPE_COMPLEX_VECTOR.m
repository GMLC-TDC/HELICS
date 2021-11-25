function v = HELICS_DATA_TYPE_COMPLEX_VECTOR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 22);
  end
  v = vInitialized;
end
