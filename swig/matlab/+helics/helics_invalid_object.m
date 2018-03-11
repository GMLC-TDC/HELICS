function v = helics_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1);
  end
  v = vInitialized;
end
