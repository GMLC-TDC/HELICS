function v = HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 120);
  end
  v = vInitialized;
end
