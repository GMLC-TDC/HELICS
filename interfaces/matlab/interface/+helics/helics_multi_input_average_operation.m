function v = helics_multi_input_average_operation()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 92);
  end
  v = vInitialized;
end
