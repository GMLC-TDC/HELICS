function v = helics_core_type_nng()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183041);
  end
  v = vInitialized;
end
