function v = helics_handle_option_connection_required()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183101);
  end
  v = vInitialized;
end
