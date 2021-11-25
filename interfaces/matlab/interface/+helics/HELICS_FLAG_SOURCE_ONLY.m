function v = HELICS_FLAG_SOURCE_ONLY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 33);
  end
  v = vInitialized;
end
