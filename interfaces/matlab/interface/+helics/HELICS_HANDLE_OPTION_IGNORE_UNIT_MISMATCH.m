function v = HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 110);
  end
  v = vInitialized;
end
