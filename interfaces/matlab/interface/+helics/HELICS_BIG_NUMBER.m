function v = HELICS_BIG_NUMBER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 130);
  end
  v = vInitialized;
end
