function v = helics_filter_type_firewall()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 92);
  end
  v = vInitialized;
end
