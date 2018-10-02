function v = helics_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107589);
  end
  v = vInitialized;
end
