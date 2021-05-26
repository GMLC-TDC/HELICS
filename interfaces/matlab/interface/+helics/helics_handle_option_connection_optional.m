function v = helics_handle_option_connection_optional()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 96);
  end
  v = vInitialized;
end
