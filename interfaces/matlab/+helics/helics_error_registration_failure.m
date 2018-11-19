function v = helics_error_registration_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183079);
  end
  v = vInitialized;
end
