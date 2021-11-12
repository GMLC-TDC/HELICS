function v = HELICS_FLAG_OBSERVER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 29);
  end
  v = vInitialized;
end
