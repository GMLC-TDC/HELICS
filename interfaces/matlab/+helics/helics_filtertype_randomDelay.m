function v = helics_filtertype_randomDelay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230883);
  end
  v = vInitialized;
end
