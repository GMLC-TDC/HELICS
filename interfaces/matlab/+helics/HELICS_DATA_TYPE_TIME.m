function v = HELICS_DATA_TYPE_TIME()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183127);
  end
  v = vInitialized;
end
