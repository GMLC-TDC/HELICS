function v = helics_filtertype_reroute()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230885);
  end
  v = vInitialized;
end
