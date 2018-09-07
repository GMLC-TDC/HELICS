function v = helics_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876555);
  end
  v = vInitialized;
end
