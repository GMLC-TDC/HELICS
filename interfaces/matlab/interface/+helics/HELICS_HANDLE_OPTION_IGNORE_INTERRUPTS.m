function v = HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 115);
  end
  v = vInitialized;
end
