function v = helics_invalid_object()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176305);
  end
  v = vInitialized;
end
