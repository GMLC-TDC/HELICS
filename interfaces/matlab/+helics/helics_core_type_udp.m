function v = helics_core_type_udp()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 7);
  end
  v = vInitialized;
end
