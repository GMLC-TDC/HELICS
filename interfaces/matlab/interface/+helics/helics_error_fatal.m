function v = helics_error_fatal()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 53);
  end
  v = vInitialized;
end
