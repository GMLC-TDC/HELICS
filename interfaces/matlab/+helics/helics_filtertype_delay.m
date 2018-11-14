function v = helics_filtertype_delay()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230882);
  end
  v = vInitialized;
end
