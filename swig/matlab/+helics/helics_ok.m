function v = helics_ok()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107587);
  end
  v = vInitialized;
end
