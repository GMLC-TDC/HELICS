function v = helics_handle_option_ignore_interrupts()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 77);
  end
  v = vInitialized;
end
