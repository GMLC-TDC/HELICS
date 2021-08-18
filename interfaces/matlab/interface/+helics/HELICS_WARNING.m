function v = HELICS_WARNING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 71);
  end
  v = vInitialized;
end
