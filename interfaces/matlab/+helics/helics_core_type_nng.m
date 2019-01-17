function v = helics_core_type_nng()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812631);
  end
  v = vInitialized;
end
