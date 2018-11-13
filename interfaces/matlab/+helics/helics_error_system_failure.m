function v = helics_error_system_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 6);
  end
  v = vInitialized;
end
