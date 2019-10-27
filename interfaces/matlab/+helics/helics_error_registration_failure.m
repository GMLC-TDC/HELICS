function v = helics_error_registration_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 48);
  end
  v = vInitialized;
end
