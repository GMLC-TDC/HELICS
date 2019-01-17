function v = helics_core_type_udp()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812629);
  end
  v = vInitialized;
end
