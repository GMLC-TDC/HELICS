function v = HELICS_HANDLE_OPTION_BUFFER_DATA()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 108);
  end
  v = vInitialized;
end
