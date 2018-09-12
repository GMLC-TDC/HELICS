function v = helics_randomDelay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183063);
  end
  v = vInitialized;
end
