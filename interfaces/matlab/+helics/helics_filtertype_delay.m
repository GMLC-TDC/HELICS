function v = helics_filtertype_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183108);
  end
  v = vInitialized;
end
