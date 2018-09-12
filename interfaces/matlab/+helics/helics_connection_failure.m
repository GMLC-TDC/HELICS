function v = helics_connection_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183034);
  end
  v = vInitialized;
end
