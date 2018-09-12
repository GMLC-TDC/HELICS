function v = helics_reroute_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183065);
  end
  v = vInitialized;
end
