function v = HELICS_OK()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 78);
  end
  v = vInitialized;
end
