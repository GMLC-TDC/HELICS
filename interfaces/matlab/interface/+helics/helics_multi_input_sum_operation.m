function v = helics_multi_input_sum_operation()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 90);
  end
  v = vInitialized;
end
