function v = helics_filtertype_reroute()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 79);
  end
  v = vInitialized;
end
