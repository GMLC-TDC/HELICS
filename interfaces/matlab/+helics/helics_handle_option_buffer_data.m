function v = helics_handle_option_buffer_data()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 73);
  end
  v = vInitialized;
end
