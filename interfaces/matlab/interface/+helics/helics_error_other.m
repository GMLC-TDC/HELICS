function v = helics_error_other()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 55);
  end
  v = vInitialized;
end
