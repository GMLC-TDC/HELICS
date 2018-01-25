function v = HELICS_INT_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 35);
  end
  v = vInitialized;
end
