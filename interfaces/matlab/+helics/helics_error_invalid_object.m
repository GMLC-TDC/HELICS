function v = helics_error_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812671);
  end
  v = vInitialized;
end
