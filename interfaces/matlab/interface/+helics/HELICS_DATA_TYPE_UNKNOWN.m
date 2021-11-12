function v = HELICS_DATA_TYPE_UNKNOWN()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 15);
  end
  v = vInitialized;
end
