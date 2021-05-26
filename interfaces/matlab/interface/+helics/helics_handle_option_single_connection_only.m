function v = helics_handle_option_single_connection_only()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 97);
  end
  v = vInitialized;
end
