function v = helics_multi_input_min_operation()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 93);
  end
  v = vInitialized;
end
