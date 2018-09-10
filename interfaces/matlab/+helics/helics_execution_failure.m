function v = helics_execution_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535364);
  end
  v = vInitialized;
end
