function v = helics_core_type_test()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183035);
  end
  v = vInitialized;
end
