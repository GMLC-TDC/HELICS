function v = helics_flag_delay_init_entry()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 38);
  end
  v = vInitialized;
end
