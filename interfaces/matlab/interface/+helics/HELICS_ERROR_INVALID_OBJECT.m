function v = HELICS_ERROR_INVALID_OBJECT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 81);
  end
  v = vInitialized;
end
