function v = helics_filtertype_randomDelay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183063);
  end
  v = vInitialized;
end
