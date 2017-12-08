function v = HELICS_RAW_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 16);
  end
  v = vInitialized;
end
