function v = helics_handle_option_multiple_connections_allowed()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 92);
  end
  v = vInitialized;
end
