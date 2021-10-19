function v = HELICS_OK()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 81);
  end
  v = vInitialized;
end
