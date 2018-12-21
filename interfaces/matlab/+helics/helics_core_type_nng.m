function v = helics_core_type_nng()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 9);
  end
  v = vInitialized;
end
