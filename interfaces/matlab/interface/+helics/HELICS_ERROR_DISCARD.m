function v = HELICS_ERROR_DISCARD()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 78);
  end
  v = vInitialized;
end
