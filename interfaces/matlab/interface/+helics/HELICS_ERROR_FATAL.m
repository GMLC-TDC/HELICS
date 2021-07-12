function v = HELICS_ERROR_FATAL()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 58);
  end
  v = vInitialized;
end
