function v = helics_reroute_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176333);
  end
  v = vInitialized;
end
