function v = HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 112);
  end
  v = vInitialized;
end
