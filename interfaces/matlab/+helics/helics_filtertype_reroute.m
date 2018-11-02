function v = helics_filtertype_reroute()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856888);
  end
  v = vInitialized;
end
