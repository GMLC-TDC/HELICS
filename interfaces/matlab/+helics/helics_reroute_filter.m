function v = helics_reroute_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1936535387);
  end
  v = vInitialized;
end
