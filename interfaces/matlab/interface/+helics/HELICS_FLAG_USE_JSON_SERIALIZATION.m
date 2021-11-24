function v = HELICS_FLAG_USE_JSON_SERIALIZATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 44);
  end
  v = vInitialized;
end
