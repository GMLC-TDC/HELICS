function v = helics_error_other()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183078);
  end
  v = vInitialized;
end
