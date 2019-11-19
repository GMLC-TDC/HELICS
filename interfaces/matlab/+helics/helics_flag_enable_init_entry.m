function v = helics_flag_enable_init_entry()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 38);
  end
  v = vInitialized;
end
