function v = HELICS_BIG_NUMBER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 133);
  end
  v = vInitialized;
end
