function v = helics_pending_init_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 21);
  end
  v = vInitialized;
end
