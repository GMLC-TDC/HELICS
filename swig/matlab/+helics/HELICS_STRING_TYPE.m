function v = HELICS_STRING_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 11);
  end
  v = vInitialized;
end
