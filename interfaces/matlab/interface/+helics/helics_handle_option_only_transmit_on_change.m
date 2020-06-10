function v = helics_handle_option_only_transmit_on_change()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 96);
  end
  v = vInitialized;
end
