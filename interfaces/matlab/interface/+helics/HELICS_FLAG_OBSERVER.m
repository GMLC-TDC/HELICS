function v = HELICS_FLAG_OBSERVER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 28);
  end
  v = vInitialized;
end
