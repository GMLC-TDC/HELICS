function v = helics_error_system_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183084);
  end
  v = vInitialized;
end
