function v = helics_flag_only_transmit_on_change()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 29);
  end
  v = vInitialized;
end
