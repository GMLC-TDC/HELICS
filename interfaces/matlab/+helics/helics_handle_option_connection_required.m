function v = helics_handle_option_connection_required()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 72);
  end
  v = vInitialized;
end
