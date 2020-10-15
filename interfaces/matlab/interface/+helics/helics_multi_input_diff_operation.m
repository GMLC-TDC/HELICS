function v = helics_multi_input_diff_operation()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 89);
  end
  v = vInitialized;
end
