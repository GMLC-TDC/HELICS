function v = helics_filtertype_custom()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183096);
  end
  v = vInitialized;
end
