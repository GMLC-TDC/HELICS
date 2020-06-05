function v = helics_multi_input_or_operation()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 83);
  end
  v = vInitialized;
end
