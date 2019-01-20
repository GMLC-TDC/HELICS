function v = helics_handle_option_buffer_data()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812695);
  end
  v = vInitialized;
end
