function v = helics_randomDelay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876581);
  end
  v = vInitialized;
end
