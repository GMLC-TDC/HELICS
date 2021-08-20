function v = HELICS_HANDLE_OPTION_CONNECTION_REQUIRED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 103);
  end
  v = vInitialized;
end
