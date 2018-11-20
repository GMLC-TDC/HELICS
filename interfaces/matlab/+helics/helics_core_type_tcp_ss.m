function v = helics_core_type_tcp_ss()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183042);
  end
  v = vInitialized;
end
