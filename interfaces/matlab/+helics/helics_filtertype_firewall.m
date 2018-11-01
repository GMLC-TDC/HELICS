function v = helics_filtertype_firewall()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095523);
  end
  v = vInitialized;
end
