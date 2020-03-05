function v = helics_error_fatal()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 65);
  end
  v = vInitialized;
end
