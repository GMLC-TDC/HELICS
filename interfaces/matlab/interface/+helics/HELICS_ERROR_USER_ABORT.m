function v = HELICS_ERROR_USER_ABORT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 72);
  end
  v = vInitialized;
end
