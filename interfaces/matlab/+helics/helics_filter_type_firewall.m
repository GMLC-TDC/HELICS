function v = helics_filter_type_firewall()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812706);
  end
  v = vInitialized;
end
