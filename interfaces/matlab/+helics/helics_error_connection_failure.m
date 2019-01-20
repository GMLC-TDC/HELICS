function v = helics_error_connection_failure()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812670);
  end
  v = vInitialized;
end
