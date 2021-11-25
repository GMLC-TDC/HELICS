function v = HELICS_DATA_TYPE_NAMED_POINT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 23);
  end
  v = vInitialized;
end
