function v = HELICS_INT_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 13);
  end
  v = vInitialized;
end
