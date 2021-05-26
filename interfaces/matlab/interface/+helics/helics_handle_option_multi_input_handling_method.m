function v = helics_handle_option_multi_input_handling_method()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 105);
  end
  v = vInitialized;
end
