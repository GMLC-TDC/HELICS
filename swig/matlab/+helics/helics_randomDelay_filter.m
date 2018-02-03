function v = helics_randomDelay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 29);
  end
  v = vInitialized;
end
