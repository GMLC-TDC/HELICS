function v = helics_flag_enable_init_entry()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812657);
  end
  v = vInitialized;
end
