function v = helics_multi_input_no_op()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 84);
  end
  v = vInitialized;
end
