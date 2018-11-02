function v = helics_error_registration_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856856);
  end
  v = vInitialized;
end
