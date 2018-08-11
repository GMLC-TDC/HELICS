function v = helics_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 2);
  end
  v = vInitialized;
end
