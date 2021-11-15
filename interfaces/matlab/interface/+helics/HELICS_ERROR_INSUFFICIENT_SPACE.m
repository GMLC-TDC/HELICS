function v = HELICS_ERROR_INSUFFICIENT_SPACE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 70);
  end
  v = vInitialized;
end
