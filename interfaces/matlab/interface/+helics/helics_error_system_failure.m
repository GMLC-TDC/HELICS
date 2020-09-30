function v = helics_error_system_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 64);
  end
  v = vInitialized;
end
