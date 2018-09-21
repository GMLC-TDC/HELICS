function v = HELICS_DATA_TYPE_NAMEDPOINT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183108);
  end
  v = vInitialized;
end
