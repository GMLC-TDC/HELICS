function v = helics_filtertype_reroute()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095521);
  end
  v = vInitialized;
end
