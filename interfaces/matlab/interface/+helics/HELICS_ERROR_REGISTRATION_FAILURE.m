function v = HELICS_ERROR_REGISTRATION_FAILURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 77);
  end
  v = vInitialized;
end
