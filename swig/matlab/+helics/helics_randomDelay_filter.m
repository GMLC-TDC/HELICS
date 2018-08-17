function v = helics_randomDelay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783864);
  end
  v = vInitialized;
end
