function v = helics_core_type_test()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812625);
  end
  v = vInitialized;
end
