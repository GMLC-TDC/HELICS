function v = helics_error_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183081);
  end
  v = vInitialized;
end
