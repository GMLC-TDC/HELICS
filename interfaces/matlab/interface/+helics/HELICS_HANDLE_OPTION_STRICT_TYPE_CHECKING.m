function v = HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 109);
  end
  v = vInitialized;
end
