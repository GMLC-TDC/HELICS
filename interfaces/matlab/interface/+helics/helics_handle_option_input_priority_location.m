function v = helics_handle_option_input_priority_location()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 100);
  end
  v = vInitialized;
end
