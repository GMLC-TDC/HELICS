function v = HELICS_FLAG_USE_JSON_SERIALIZATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 43);
  end
  v = vInitialized;
end
