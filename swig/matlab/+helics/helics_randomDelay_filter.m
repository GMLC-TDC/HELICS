function v = helics_randomDelay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107615);
  end
  v = vInitialized;
end
