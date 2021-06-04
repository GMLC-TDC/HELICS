function v = helics_multi_input_and_operation()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 88);
  end
  v = vInitialized;
end
