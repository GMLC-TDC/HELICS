function v = helics_filtertype_reroute()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183113);
  end
  v = vInitialized;
end
